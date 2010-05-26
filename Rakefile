require 'rake'
require 'open3'
require 'benchmark'

task :default => [:compile, :test]

task :clean do
	sh 'make clean'
	sh 'make -C tests/unit clean'
end

task :compile do
	sh 'make libneko neko'
end

task :test => [:unit_test, :neko_test]

task :unit_test do
	sh 'make -C tests/unit test'
end

def neko_environment
	"LD_LIBRARY_PATH=bin:/usr/lib/llvm/lib:${LD_LIBRARY_PATH} NEKOPATH=boot:bin"
end

def neko_command(param)
	"#{neko_environment} bin/neko #{param}"
end

task :neko, [:param] => [:compile] do |t, args|
	sh neko_command(args.param)
end

rule '.n' => '.neko' do |t|
	sh "nekoc #{t.source}"
end

TEST_SOURCE = FileList['tests/code/*.neko']
TEST_BINARIES = TEST_SOURCE.ext('n')

#make tasks for running tests
TEST_SOURCE.each { |f|
	Rake::Task.define_task("run_" + File.basename(f, '.neko') => [:compile, f.ext('n')]) do |t|
		sh neko_command('--jit --llvm-jit --no-llvm-optimizations --dump-neko --dump-llvm ' + f.ext('n'))
	end

	Rake::Task.define_task("trace_" + File.basename(f, '.neko') => [:compile, f.ext('n')]) do |t|
		sh "#{neko_environment} gdb --batch --eval-command 'run' --eval-command 'bt' --args bin/neko --jit --llvm-jit --no-llvm-optimizations --dump-neko --dump-llvm #{f.ext('n')}"
	end
}

def table_header
	puts <<EOF
+-------------+-------------+-------------+-------------+-------------+------+
|    test name|       interp|      x86 jit|     llvm jit|     llvm jit|status|
|             |             |             |       no opt|     with opt|      |
+-------------+-------------+-------------+-------------+-------------+------+
EOF
end

def one_neko_test(speeds, param, file)
	result = "";
	speeds << Benchmark.realtime {
		Open3.popen3(neko_command("#{param} #{file}")) { |stdin, stdout, stderr, wait_thr|
			result = stdout.readlines.join('\n');
		}
	}
	result
end

task :neko_test => TEST_BINARIES do |t|
	all_test_passed = true

	table_header
	column_sizes = [13, 13, 13, 13, 13, 6]
	columns = ["--no-jit", "--jit --no-llvm-jit", "--jit --llvm-jit --no-llvm-optimizations", "--jit --llvm-jit --llvm-optimizations"];
	t.prerequisites.each {|f|
		#results
		printf "|%#{column_sizes[0]}s|", File.basename(f)
		speeds = []
		results = []
		columns.each_with_index {|param, i|
			result = one_neko_test(speeds, param, f)
			printf "%#{column_sizes[i+1]}s|", result
			results << result
		}
		printf "%#{column_sizes[-1]}s|", (results.uniq.length == 1) ? "OK" : "Error"
		all_test_passed = all_test_passed && (results.uniq.length == 1)
		print "\n"

		#speeds
		printf "|%#{column_sizes[0]}s|", ""
		speeds.each_with_index{|s, q|
			printf "%#{column_sizes[q+1]-2}dms|", (s * 1000).to_i
		}
		printf "%#{column_sizes[-1]}s|", ""
		print "\n"

		#closing line
		puts "+-------------+-------------+-------------+-------------+-------------+------+"
	}

	raise "Some tests has failed" if !all_test_passed
end
