#include <pong/app.h>
#include <polar/system/asset.h>
#include <polar/system/event.h>
#include <polar/system/integrator.h>
#include <polar/system/renderer/gl32.h>
#include <polar/component/model.h>

namespace pong {
	app::app(polar::core::polar &engine) {
		using namespace polar;
		engine.addstate("root", [] (core::polar *engine, core::state &st) {
			st.addsystem<system::asset>();
			st.addsystem<system::event>();
			st.addsystem<system::integrator>();
			st.addsystem_as<system::renderer::base, system::renderer::gl32, std::vector<std::string>>({"2d"});
		});
		engine.addstate("game", [] (core::polar *engine, core::state &st) {
			IDType leftPaddle, rightPaddle, ball;
			st.dtors.emplace_back(engine->addobject(&leftPaddle));
			st.dtors.emplace_back(engine->addobject(&rightPaddle));
			st.dtors.emplace_back(engine->addobject(&ball));

			engine->addcomponent<component::model, component::model::TrianglesType>(leftPaddle, {
				{
					{ 0, 0, 0 },
					{ 1, 0, 0 },
					{ 0, 1, 0 }
				}
			});
		});

		engine.run("root");
	}
}
