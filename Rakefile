task :default => [:compile, :test]

task :compile do
	sh 'make libneko neko'
end

task :test do
	neko "../test.n"
end

def neko(param)
	sh "LD_LIBRARY_PATH=bin:/usr/lib/llvm/lib:${LD_LIBRARY_PATH} NEKOPATH=boot:bin bin/neko #{param}"
end
