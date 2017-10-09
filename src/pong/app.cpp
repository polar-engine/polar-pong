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

			st.add_system<system::asset>();
			st.add_system<system::event>();
			st.add_system<system::input>();
			st.add_system<system::integrator>();
			st.add_system_as<system::renderer::base, system::renderer::gl32>(std::vector<std::string>{"2d"});

			engine->transition = "forward";
		});
		engine.addstate("game", [] (core::polar *engine, core::state &st) {
			IDType leftPaddle, rightPaddle, ball;
			st.dtors.emplace_back(engine->add_object(&leftPaddle));
			st.dtors.emplace_back(engine->add_object(&rightPaddle));
			st.dtors.emplace_back(engine->add_object(&ball));

			auto model = std::make_shared<component::model>(GeometryType::TriangleStrip, component::model::PointsType{
				{ -1, -1, 0 },
				{  1, -1, 0 },
				{ -1,  1, 0 },
				{  1,  1, 0 }
			});

			engine->insert_component(leftPaddle,  model);
			engine->insert_component(rightPaddle, model);
			engine->insert_component(ball,        model);

			engine->add_component<component::position>(leftPaddle,  Point3(-0.925, 0, 0));
			engine->add_component<component::position>(rightPaddle, Point3( 0.925, 0, 0));
			engine->add_component<component::position>(ball);

			engine->add_component<component::scale>(leftPaddle,  Point3(0.025, 0.2,  0));
			engine->add_component<component::scale>(rightPaddle, Point3(0.025, 0.2,  0));
			engine->add_component<component::scale>(ball,        Point3(0.02,  0.02, 0));

			engine->add_component<component::bounds>(leftPaddle,  Point3(-1, -1, 0),
			                                                      Point3( 2,  2, 0));
			engine->add_component<component::bounds>(rightPaddle, Point3(-1, -1, 0),
			                                                      Point3( 2,  2, 0));
			engine->add_component<component::bounds>(ball,        Point3(-1, -1, 0),
			                                                      Point3( 2,  2, 0));

			engine->get_component<component::position>(ball)->pos.derivative() = Point3(-0.5, -0.1, 0);

			auto inputM = engine->get_system<system::input>().lock();
			st.dtors.emplace_back(inputM->on(key_t::Escape, [engine] (key_t) { engine->quit(); }));

			const Decimal speed = 1.5;

			auto onKey = [engine, speed, leftPaddle] (key_t k) {
				auto paddle = engine->get_component<component::position>(leftPaddle);
				auto &y = paddle->pos.derivative()->y;
				switch(k) {
					case key_t::Up:
						y += speed;
						break;
					case key_t::Down:
						y -= speed;
						break;
					default:
						break;
				}
			};
			auto afterKey = [engine, speed, leftPaddle] (key_t k) {
				auto paddle = engine->get_component<component::position>(leftPaddle);
				auto &y = paddle->pos.derivative()->y;
				switch(k) {
					case key_t::Up:
						y -= speed;
						break;
					case key_t::Down:
						y += speed;
						break;
					default:
						break;
				}
			};
			st.dtors.emplace_back(inputM->on(key_t::Up,   onKey));
			st.dtors.emplace_back(inputM->on(key_t::Down, onKey));
			st.dtors.emplace_back(inputM->after(key_t::Up,   afterKey));
			st.dtors.emplace_back(inputM->after(key_t::Down, afterKey));
		});

		engine.run("root");
	}
}
