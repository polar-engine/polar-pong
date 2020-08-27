#include <polar/asset/model.h>
#include <polar/component/model.h>
#include <polar/component/phys.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/math/types.h>
#include <polar/support/action/keyboard.h>
#include <polar/support/action/mouse.h>
#include <polar/support/input/key.h>
#include <polar/support/phys/detector/box.h>
#include <polar/support/phys/responder/rigid.h>
#include <polar/support/phys/responder/stat.h>
#include <polar/system/action.h>
#include <polar/system/asset.h>
#include <polar/system/integrator.h>
#include <polar/system/phys.h>
#include <polar/system/renderer/gl32.h>
#include <polar/system/sched.h>
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
			st.add<system::sched>();
			st.add<system::action>();
			st.add<system::integrator>();
			st.add<system::phys>();
			st.add_as<system::renderer::base, system::renderer::gl32>(std::vector<std::string>{"2d"});

			engine->transition = "forward";
		});
		engine.add("game", [](core::polar *engine, core::state &st) {
			core::ref left_paddle, right_paddle, ball;
			st.keep(left_paddle  = engine->add());
			st.keep(right_paddle = engine->add());
			st.keep(ball         = engine->add());

			auto assetM = engine->get<polar::system::asset>().lock();
			auto as = assetM->get<polar::asset::model>("box");
			auto model = std::make_shared<polar::component::model>(as);

			engine->insert(left_paddle, model);
			engine->insert(right_paddle, model);
			engine->insert(ball, model);

			engine->add<component::position>(left_paddle, math::point3(-0.925, 0, 0));
			engine->add<component::position>(right_paddle, math::point3(0.925, 0, 0));
			engine->add<component::position>(ball);

			auto scale = std::make_shared<component::scale>(math::point3(0.025, 0.2, 0));

			engine->insert(left_paddle,  scale);
			engine->insert(right_paddle, scale);
			engine->add<component::scale>(ball, math::point3(0.02, 0.02, 0));

			auto phys = std::make_shared<component::phys>(detector::box(), responder::stat());

			engine->insert(left_paddle, phys);
			engine->insert(right_paddle, phys);
			engine->add<component::phys>(ball, detector::box(), responder::rigid());

			engine->get<component::position>(ball)->pos.derivative() = math::point3(-0.5, -0.1, 0);

			namespace kb    = support::action::keyboard;
			namespace mouse = support::action::mouse;

			struct a_left_paddle  : system::action::analog {};
			struct a_right_paddle : system::action::analog {};
			struct a_quit_game    : system::action::digital {};

			auto action = engine->get<system::action>().lock();

			const math::decimal speed = 1.5;
			auto on_axis_paddle = [engine, speed](core::ref paddle, math::decimal delta) {
				auto position = engine->get<component::position>(paddle);
				position->pos.derivative()->y = delta * speed;
			};

			st.keep(action->bind<a_left_paddle >(std::bind(on_axis_paddle, left_paddle,  std::placeholders::_2)));
			st.keep(action->bind<a_right_paddle>(std::bind(on_axis_paddle, right_paddle, std::placeholders::_2)));
			st.keep(action->bind<a_quit_game>(lifetime::on, [engine](auto) { engine->quit(); }));

			st.keep(action->bind<kb::key<key::W>,      a_left_paddle >(lifetime::when,  1));
			st.keep(action->bind<kb::key<key::S>,      a_left_paddle >(lifetime::when, -1));
			st.keep(action->bind<kb::key<key::Up>,     a_right_paddle>(lifetime::when,  1));
			st.keep(action->bind<kb::key<key::Down>,   a_right_paddle>(lifetime::when, -1));
			st.keep(action->bind<kb::key<key::Escape>, a_quit_game   >(lifetime::after));
			st.keep(action->bind<mouse::motion_y, a_left_paddle>());

			st.keep(action->bind<kb::key<key::H>>(lifetime::when, [](auto) {
				polar::log()->notice("pong", "hello world");
			}));
		});

		engine.run("root");
	}
} // namespace pong
