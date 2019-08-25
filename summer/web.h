#ifndef _SUMMER_MODULE_WEB_H
#define _SUMMER_MODULE_WEB_H

#include "../summer.h"

#include <vector>

namespace summer::modules::web {

	class controller {};

	class web_module : public summer::module_base<web_module, controller> {
	public:
		template <typename ContextSupport>
		void initialize(ContextSupport& context, std::vector<std::string> const& args) {
			std::cout << "web module initialize" << std::endl;
		}

		void register_type(std::shared_ptr<controller> web_controller) {
			std::cout << "web controller registered!" << std::endl;
		}

	};

}

#endif
