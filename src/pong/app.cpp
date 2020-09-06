#include <polar/asset/model.h>
#include <polar/component/framebuffer.h>
#include <polar/component/material.h>
#include <polar/component/model.h>
#include <polar/component/phys.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/component/stage.h>
#include <polar/math/types.h>
#include <polar/support/action/keyboard.h>
#include <polar/support/action/mouse.h>
#include <polar/support/input/key.h>
#include <polar/support/phys/detector/box.h>
#include <polar/support/phys/responder/rigid.h>
#include <polar/support/phys/responder/stat.h>
#include <polar/system/action.h>
#include <polar/system/asset.h>
#include <polar/system/font.h>
#include <polar/system/integrator.h>
#include <polar/system/opengl/debug.h>
#include <polar/system/opengl/framebuffer.h>
#include <polar/system/opengl/model.h>
#include <polar/system/opengl/renderer.h>
#include <polar/system/opengl/stage.h>
#include <polar/system/opengl/texture.h>
#include <polar/system/opengl/window.h>
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
			st.add<system::asset>();
			st.add<system::action>();
			st.add<system::font>();
			st.add<system::integrator>();
			st.add<system::opengl::debug>();
			st.add<system::opengl::framebuffer>();
			st.add<system::opengl::model>();
			st.add<system::opengl::renderer>();
			st.add<system::opengl::stage>();
			st.add<system::opengl::texture>();
			st.add<system::opengl::window>();
			st.add<system::phys>();
			st.add<system::renderer::gl32>();
			st.add<system::sched>();

			auto assetM = engine->get<system::asset>().lock();
			auto box_asset     = assetM->get<asset::model>("box");
			auto diffuse_asset = assetM->get<asset::image>("diffuse1");

			core::ref win             = engine->add();
			core::ref fb_main         = engine->add();
			core::ref stage_main      = engine->add();
			core::ref stage_chroma    = engine->add();
			core::ref diffuse         = engine->add();
			core::ref viewport_chroma = engine->add();
			core::ref ball            = engine->add();
			core::ref left_paddle     = engine->add();
			core::ref right_paddle    = engine->add();

			engine->add<component::window>(win, math::point2i(1280, 1280));
			engine->add<component::framebuffer>(fb_main, win);
			engine->add<component::stage>(stage_main, "2d", fb_main);
			engine->add<component::stage>(stage_chroma, "chroma", win);

			engine->add<component::texture>(diffuse, diffuse_asset);

			engine->add<component::model>(viewport_chroma, box_asset, stage_chroma);
			engine->add<component::material>(viewport_chroma, fb_main);

			engine->add<component::model>(ball, box_asset, stage_main);
			engine->add<component::material>(ball, diffuse);
			engine->add<component::position>(ball)->pos.derivative() = math::point3(-0.5, -0.1, 0);
			engine->add<component::scale>(ball, math::point3(0.2, 0.2, 0));
			engine->add<component::phys>(ball, detector::box(), responder::rigid());

			engine->add<component::model>(left_paddle, box_asset, stage_main);
			engine->add<component::position>(left_paddle, math::point3(-0.925, 0, 0));
			engine->add<component::scale>(left_paddle, math::point3(0.025, 0.2, 0));
			engine->add<component::phys>(left_paddle, detector::box(), responder::stat());

			engine->add<component::model>(right_paddle, box_asset, stage_main);
			engine->add<component::position>(right_paddle, math::point3( 0.925, 0, 0));
			engine->add<component::scale>(right_paddle, math::point3(0.025, 0.2, 0));
			engine->add<component::phys>(right_paddle, detector::box(), responder::stat());

			st.keep(win);
			st.keep(fb_main);
			st.keep(stage_main);
			st.keep(stage_chroma);
			st.keep(diffuse);
			st.keep(viewport_chroma);
			st.keep(ball);
			st.keep(left_paddle);
			st.keep(right_paddle);



			namespace kb    = support::action::keyboard;
			namespace mouse = support::action::mouse;

			struct a_left_paddle  : system::action::analog {};
			struct a_right_paddle : system::action::analog {};
			struct a_quit_game    : system::action::digital {};

			auto action = engine->get<system::action>().lock();

			const math::decimal speed = 1.5;
			auto on_axis_paddle = [engine, speed](core::ref paddle, math::decimal delta) {
				auto position = engine->mutate<component::position>(paddle);
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
			//st.keep(action->bind<mouse::motion_y, a_left_paddle>());

			st.keep(action->bind<kb::key<key::H>>(lifetime::when, [](auto) {
				polar::log()->notice("pong", "hello world");
			}));

			st.keep(action->bind<kb::key<key::R>>(lifetime::on, [engine, win](auto) {
				engine->mutate<component::window>(win)->size.x = 1500;
			}));
		});

		engine.run("root");
	}
} // namespace pong
