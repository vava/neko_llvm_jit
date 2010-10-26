require 'rake'
require 'open3'
require 'benchmark'

task :default => [:compile, :test]

task :clean do
	sh 'make clean'
	sh 'make -C tests/unit clean'
end

VM_FILES = FileList['vm/stats.c', 'vm/main.c']
LLVM_JIT_FILES = FileList['vm/llvm/*.cpp', 'vm/llvm/*.h']
STD_FILES = FileList['libs/std/*.c']
LIBNEKO_FILES = FileList['vm/*.c', 'Makefile']
LIBNEKO = 'bin/libneko.so'
COMPILER_FILES = FileList['src/neko/*.nml', 'src/nekoml/*.nml']
TOOLS_FILES = FileList['src/core/*.nml', 'src/tools/*.nml', 'src/tools/install.neko', 'boot/nekoc.n', 'boot/nekoml.n']

file LIBNEKO => FileList[LLVM_JIT_FILES, LIBNEKO_FILES] do
	sh 'make libneko'
end
task :make_libneko => LIBNEKO

file 'bin/neko' => FileList[VM_FILES, LLVM_JIT_FILES, LIBNEKO] do
	sh 'make neko'
end
task :make_neko => 'bin/neko'

file 'bin/std.ndll' => FileList[STD_FILES, LIBNEKO] do
	sh 'make std'
end
task :make_std => 'bin/std.ndll'

file 'boot/nekoc.n' => COMPILER_FILES do
	sh 'make compiler'
end
task :make_compiler => 'boot/nekoc.n'

file 'bin/nekoc' => TOOLS_FILES do
	sh 'SKIP_UNAVAILABLE="true" make libs'
end
task :make_libs => 'bin/nekoc'

task :compile => [:make_neko, :make_std, :make_libs]

task :test => [:unit_test, :neko_test]

task :unit_test do
	sh 'make -C tests/unit test'
end

def neko_environment
	"LD_LIBRARY_PATH=bin:${LD_LIBRARY_PATH} NEKOPATH=boot:bin"
end

def neko_command(param)
	"#{neko_environment} bin/neko #{param}"
end

task :neko_trace, [:param] => [:compile] do |t, args|
	sh "#{neko_environment} gdb --batch --eval-command 'run' --eval-command 'bt' --args bin/neko --jit --llvm-jit --no-llvm-optimizations --dump-neko --dump-llvm #{args.param}"
end

task :neko_run, [:param] => [:compile] do |t, args|
	sh "#{neko_environment} bin/neko --jit --llvm-jit #{args.param}"
end

task :neko, [:param] => [:compile] do |t, args|
	sh neko_command(args.param)
end

rule '.n' => '.neko' do |t|
	sh "nekoc #{t.source}"
end

rule '.n' => '.hx' do |t|
	sh "haxe -neko #{t.name} -main #{t.source.ext('').gsub('/', '.')}"
end

TEST_SOURCE = FileList['tests/code/*.neko', 'tests/code/*.hx']
TEST_BINARIES = TEST_SOURCE.ext('n')

#make tasks for running tests
TEST_SOURCE.each { |f|
	Rake::Task.define_task("run_" + File.basename(f, '.neko') => [:compile, f.ext('n')]) do |t|
		sh neko_command('--jit --llvm-jit --no-llvm-optimizations --dump-neko --dump-llvm ' + f.ext('n'))
	end

	Rake::Task.define_task("run_opt_" + File.basename(f, '.neko') => [:compile, f.ext('n')]) do |t|
		sh neko_command('--jit --llvm-jit --llvm-optimizations --dump-neko --dump-llvm ' + f.ext('n'))
	end

	Rake::Task.define_task("trace_" + File.basename(f, '.neko') => [:compile, f.ext('n')]) do |t|
		sh "#{neko_environment} gdb --batch --eval-command 'run' --eval-command 'bt' --args bin/neko --jit --llvm-jit --no-llvm-optimizations --dump-neko --dump-llvm #{f.ext('n')}"
	end
}

class Table
	def initialize(headers)
		@headers = headers
		draw_hr
		line do |l|
			@headers.each do |h|
				l << h[:name]
			end
		end
	end

	def line(&block)
		current = []
		left_lines = 0
		column_num = 0
		draw = proc {|*args| draw_cell(*args)}
		new_class = Class.new do
			define_method(:<<) do |*args|
				c = args.shift
				cell_lines = c.split("\n")
				first_line = cell_lines.shift
				current << cell_lines
				draw.call(column_num, first_line)
				column_num += 1
				left_lines = [left_lines, cell_lines.length].max
			end
		end

		yield new_class.new

		print "|\n"

		left_lines.times do
			current.each_with_index do |v, i|
				s = v.shift
				draw_cell(i, s || '')
			end
			print "|\n"
		end
		draw_hr
	end
private
	def align_left(s, width)
		stripped = strip_escape_symbols(s)
		return s + ' ' * [(width - stripped.length), 0].max
	end

	def align_right(s, width)
		stripped = strip_escape_symbols(s)
		return ' ' * [(width - stripped.length), 0].max + s
	end

	def strip_escape_symbols(s)
		s.gsub(/\e\[[0-9;]*[A-Za-z]/, '')
	end


	def draw_cell(i, s)
		print "|"
		width = @headers[i][:width]
		stripped = strip_escape_symbols(s)
		print (( stripped =~ /^-?[0-9]/) ? align_right(s, width) : align_left(s, width))
	end

	def draw_hr
		@headers.each do |h|
			print "+", "-" * h[:width]
		end
		print "+\n"
	end
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

	headers = [
			   {
				   :name => "test name",
				   :width => 15,
			   },
			   {
				   :name => "interp",
				   :width => 13,
				   :options => "--no-jit"
			   },
			   {
				   :name => "x86 jit",
				   :width => 13,
				   :options => "--jit --no-llvm-jit"
			   },
			   {
				   :name => "llvm jit\nno opt",
				   :width => 13,
				   :options => "--jit --llvm-jit --no-llvm-optimizations"
			   },
			   {
				   :name => "llvm jit\nwith opt",
				   :width => 13,
				   :options => "--jit --llvm-jit --llvm-optimizations"
			   },
			   {
				   :name => "status",
				   :width => 6,
			   },
			  ]

	table = Table.new(headers)

	t.prerequisites.each {|f|
		table.line {|l|
			l << File.basename(f)
			results = []
			headers.each {|h|
				if (h[:options])
					ms = Benchmark.realtime {
						Open3.popen3(neko_command("#{h[:options]} #{f}")) { |stdin, stdout, stderr, wait_thr|
							results << stdout.readlines.join("\n");
						}
					}
					l << (results[-1] + "\n" + (ms * 1000).to_i.to_s + 'ms')
				end
			}
			passed = (results.uniq.length == 1)
			l << ((passed) ? "OK" : "Error")
			all_test_passed = all_test_passed && passed
		}
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

		f << "#define REPEAT_MACRO_0_TO_0(M) M(0)\n"
		f << "#define REPEAT_MACRO_0_TO_1(M) REPEAT_MACRO_0_TO_0(M) M(1)\n"
		(2..args.num).each do |i|
			f << "#define REPEAT_MACRO_0_TO_#{i}(M) REPEAT_MACRO_0_TO_#{i-1}(M) M(#{i})\n"
		end

		f << "#define REPEAT_MACRO_1_TO_1(M) M(1)\n"
		(2..args.num).each do |i|
			f << "#define REPEAT_MACRO_1_TO_#{i}(M) REPEAT_MACRO_1_TO_#{i-1}(M) M(#{i})\n"
		end

		f << "#define REPEAT_LIST_MACRO_0(M) \n"
		f << "#define REPEAT_LIST_MACRO_1(M) M(1)\n"
		(2..args.num).each do |i|
			f << "#define REPEAT_LIST_MACRO_#{i}(M) REPEAT_LIST_MACRO_#{i-1}(M), M(#{i})\n"
		end
	end
end

file 'vm/llvm/repeat.h' => :make_repeat

