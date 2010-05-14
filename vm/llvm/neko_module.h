#include "neko_function.h"

#include "llvm/Module.h"

namespace neko {
	class Module {
	public:
		Module(neko_module const * m);

		typedef std::vector<Function> functions_container;
		typedef functions_container::iterator iterator;
		typedef functions_container::const_iterator const_iterator;
		typedef functions_container::value_type value_type;
		typedef functions_container::size_type size_type;

		iterator begin() { return functions.begin(); }
		const_iterator begin() const { return functions.begin(); }
		iterator end() { return functions.end(); }
		const_iterator end() const { return functions.end(); }
		size_type size() const { return functions.size(); }

		std::string const & getName() const { return name; }

		void neko_dump(std::string const & indent = "") const;
	private:
		const NekoCodeContainer code_container;
		functions_container functions;
		std::string name;
	};
}
