#include <pong/app.h>
#include <polar/system/asset.h>
#include <polar/system/event.h>
#include <polar/system/input.h>
#include <polar/system/integrator.h>
#include <polar/system/renderer/gl32.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/component/bounds.h>
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

			auto model = std::make_shared<component::model>(GeometryType::TriangleStrip, component::model::PointsType{
				{ -1, -1, 0 },
				{  1, -1, 0 },
				{ -1,  1, 0 },
				{  1,  1, 0 }
			});

			engine->insertcomponent(leftPaddle, model);
			engine->addcomponent<component::position>(leftPaddle, Point3(-0.925, 0, 0));
			engine->addcomponent<component::scale>(leftPaddle, Point3(0.025, 0.2, 0));

			engine->insertcomponent(rightPaddle, model);
			engine->addcomponent<component::position>(rightPaddle, Point3(0.925, 0, 0));
			engine->addcomponent<component::scale>(rightPaddle, Point3(0.025, 0.2, 0));

			engine->insertcomponent(ball, model);
			engine->addcomponent<component::position>(ball);
			engine->addcomponent<component::scale>(ball, Point3(0.02, 0.02, 0));

			engine->getcomponent<component::position>(ball)->pos.derivative() = Point3(-0.5, -0.1, 0);

			//engine->addcomponent<component::bounds>(leftPaddle,
			//                                        Point3(-0.925, 0, 0),

			auto inputM = engine->getsystem<system::input>().lock();
			st.dtors.emplace_back(inputM->on(key_t::Escape, [engine] (key_t) { engine->quit(); }));
		});

		engine.run("root");
	}
}
