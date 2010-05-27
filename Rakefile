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

REPETETION = 30

#c++ helpers
task :make_repeat, [:num] do |t, args|
	args.with_defaults :num => REPETETION
	File.open('vm/llvm/repeat.h', "w") do |f|
		f << "# This file is auto generated\n"
        f << "# Do not change it by hands, instead run $ rake make_repeat\n"
		f << "\n"
		f << "#define REPEAT_0(x)\n"
		f << "#define REPEAT_1(x) x\n"
		(2..args.num).each do |i|
			f << "#define REPEAT_#{i}(x) REPEAT_#{i-1}(x), x\n"
		end
	end
end

file 'vm/llvm/repeat.h' => :make_repeat

task :make_repeat, [:num] do |t, args|
	args.with_defaults :num => REPETETION
	File.open('vm/llvm/repeat.h', "w") do |f|
		f << "// This file is auto generated\n"
        f << "// Do not change it by hands, instead run $ rake make_repeat\n"
		f << "\n"
		f << "#define REPEAT_0(x)\n"
		f << "#define REPEAT_1(x) x\n"
		(2..args.num).each do |i|
			f << "#define REPEAT_#{i}(x) REPEAT_#{i-1}(x), x\n"
		end

		f << "#define REPEAT_MACRO_0(M) M(0)\n"
		f << "#define REPEAT_MACRO_1(M) REPEAT_MACRO_0(M) M(1)\n"
		(2..args.num).each do |i|
			f << "#define REPEAT_MACRO_#{i}(M) REPEAT_MACRO_#{i-1}(M) M(#{i})\n"
		end

		f << "#define REPEAT_LIST_MACRO_0(M) \n"
		f << "#define REPEAT_LIST_MACRO_1(M) M(1)\n"
		(2..args.num).each do |i|
			f << "#define REPEAT_LIST_MACRO_#{i}(M) REPEAT_LIST_MACRO_#{i-1}(M), M(#{i})\n"
		end
	end
end

file 'vm/llvm/repeat.h' => :make_repeat

