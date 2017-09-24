#include <pong/app.h>
#include <polar/system/asset.h>
#include <polar/system/event.h>
#include <polar/system/input.h>
#include <polar/system/integrator.h>
#include <polar/system/renderer/gl32.h>
#include <polar/component/model.h>
#include <polar/support/input/key.h>

namespace pong {
	app::app(polar::core::polar &engine) {
		using namespace polar;
		using key_t = support::input::key;

		engine.addstate("root", [] (core::polar *engine, core::state &st) {
			st.transitions.emplace("forward", Transition{ Push("game") });

			st.addsystem<system::asset>();
			st.addsystem<system::event>();
			st.addsystem<system::input>();
			st.addsystem<system::integrator>();
			st.addsystem_as<system::renderer::base, system::renderer::gl32>(std::vector<std::string>{"2d"});

			engine->transition = "forward";
		});
		engine.addstate("game", [] (core::polar *engine, core::state &st) {
			IDType leftPaddle, rightPaddle, ball;
			st.dtors.emplace_back(engine->addobject(&leftPaddle));
			st.dtors.emplace_back(engine->addobject(&rightPaddle));
			st.dtors.emplace_back(engine->addobject(&ball));

			engine->addcomponent<component::model>(leftPaddle, GeometryType::TriangleStrip, component::model::PointsType{
				{ -0.95, -0.2, 0 },
				{ -0.90, -0.2, 0 },
				{ -0.95,  0.2, 0 },
				{ -0.90,  0.2, 0 }
			});

			auto inputM = engine->getsystem<system::input>().lock();
			st.dtors.emplace_back(inputM->on(key_t::Escape, [engine] (key_t) { engine->quit(); }));
		});

		engine.run("root");
	}
}
