#include "function.h"

class ModuleCodeBase {
public:
	ModuleCodeBase(neko_module const * m);

	typedef std::map<int, Function> functions_container;
	typedef functions_container::iterator iterator;
	typedef functions_container::const_iterator const_iterator;

	iterator begin() { return functions.begin(); }
	const_iterator begin() const { return functions.begin(); }
	iterator end() { return functions.end(); }
	const_iterator end() const { return functions.end(); }

	void neko_dump(std::string const & indent = "") const;
private:
	const NekoCodeContainer code_container;
	functions_container functions;
};
