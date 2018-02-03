#include <polar/component/model.h>
#include <polar/component/phys.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/support/input/key.h>
#include <polar/support/phys/detector/box.h>
#include <polar/support/phys/responder/rigid.h>
#include <polar/support/phys/responder/stat.h>
#include <polar/system/asset.h>
#include <polar/system/event.h>
#include <polar/system/input.h>
#include <polar/system/action.h>
#include <polar/system/integrator.h>
#include <polar/system/phys.h>
#include <polar/system/renderer/gl32.h>
#include <polar/system/keyboard.h>
#include <pong/app.h>

namespace pong {
	app::app(polar::core::polar &engine) {
		using namespace polar;
		using namespace support::phys;
		using key = support::input::key;
		using lifetime = support::action::lifetime;

		engine.add("root", [](core::polar *engine, core::state &st) {
			st.transitions.emplace("forward", Transition{Push("game")});

			st.add<system::asset>();
			st.add<system::event>();
			st.add<system::input>();
			st.add<system::action>();
			st.add<system::keyboard>();
			st.add<system::integrator>();
			st.add<system::phys>();
			st.add_as<system::renderer::base, system::renderer::gl32>(
			    std::vector<std::string>{"2d"});

			engine->transition = "forward";
		});
		engine.add("game", [](core::polar *engine, core::state &st) {
			IDType left_paddle, right_paddle, ball;
			st.dtors.emplace_back(engine->add(left_paddle));
			st.dtors.emplace_back(engine->add(right_paddle));
			st.dtors.emplace_back(engine->add(ball));

			auto model = std::make_shared<component::model>(
			    GeometryType::TriangleStrip,
			    component::model::point_vec{
			        {-1, -1, 0}, {1, -1, 0}, {-1, 1, 0}, {1, 1, 0}});

			engine->insert(left_paddle, model);
			engine->insert(right_paddle, model);
			engine->insert(ball, model);

			engine->add<component::position>(left_paddle, Point3(-0.925, 0, 0));
			engine->add<component::position>(right_paddle, Point3(0.925, 0, 0));
			engine->add<component::position>(ball);

			engine->add<component::scale>(left_paddle, Point3(0.025, 0.2, 0));
			engine->add<component::scale>(right_paddle, Point3(0.025, 0.2, 0));
			engine->add<component::scale>(ball, Point3(0.02, 0.02, 0));

			auto phys = std::make_shared<component::phys>(detector::box(),
			                                              responder::stat());

			engine->insert(left_paddle, phys);
			engine->insert(right_paddle, phys);
			engine->add<component::phys>(ball, detector::box(),
			                             responder::rigid());

			engine->get<component::position>(ball)->pos.derivative() =
			    Point3(-0.5, -0.1, 0);

			auto action = engine->get<system::action>().lock();
			auto kb = engine->get<system::keyboard>().lock();

			const Decimal speed = 1.5;
			auto on_axis_paddle = [engine, speed](IDType paddle, Decimal delta) {
				auto position = engine->get<component::position>(paddle);
				position->pos.derivative()->y = delta * speed;
			};

			auto a_left_paddle  = action->analog();
			auto a_right_paddle = action->analog();
			auto a_quit_game    = action->digital();

			st.dtors.emplace_back(action->bind(a_left_paddle,  std::bind(on_axis_paddle, left_paddle,  std::placeholders::_1)));
			st.dtors.emplace_back(action->bind(a_right_paddle, std::bind(on_axis_paddle, right_paddle, std::placeholders::_1)));
			st.dtors.emplace_back(action->bind(lifetime::on, a_quit_game, [engine] { engine->quit(); }));

			st.dtors.emplace_back(action->bind(lifetime::when,  kb->action(key::W),      a_left_paddle,   1));
			st.dtors.emplace_back(action->bind(lifetime::when,  kb->action(key::S),      a_left_paddle,  -1));
			st.dtors.emplace_back(action->bind(lifetime::when,  kb->action(key::Up),     a_right_paddle,  1));
			st.dtors.emplace_back(action->bind(lifetime::when,  kb->action(key::Down),   a_right_paddle, -1));
			st.dtors.emplace_back(action->bind(lifetime::after, kb->action(key::Escape), a_quit_game));

			//st.keep(action->bind(lifetime::when,  kb->action(key::W),      a_left_paddle,   1));
			//st.keep(action->bind(lifetime::when,  kb->action(key::S),      a_left_paddle,  -1));
			//st.keep(action->bind(lifetime::when,  kb->action(key::Up),     a_right_paddle,  1));
			//st.keep(action->bind(lifetime::when,  kb->action(key::Down),   a_right_paddle, -1));
			//st.keep(action->bind(lifetime::after, kb->action(key::Escape), a_quit_game));

			auto a_hello_world = action->digital();
			st.dtors.emplace_back(action->bind(lifetime::on, a_hello_world, [] { debugmanager()->info("hello world"); }));
			st.dtors.emplace_back(action->bind(lifetime::when, kb->action(key::H), a_hello_world));
		});

		engine.run("root");
	}
} // namespace pong
