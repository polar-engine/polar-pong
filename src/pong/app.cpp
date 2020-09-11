#include <polar/asset/model.h>
#include <polar/component/framebuffer.h>
#include <polar/component/material.h>
#include <polar/component/model.h>
#include <polar/component/phys.h>
#include <polar/component/position.h>
#include <polar/component/scale.h>
#include <polar/component/scene.h>
#include <polar/component/stage.h>
#include <polar/math/constants.h>
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
			st.add<system::sched>();

			auto assetM = engine->get<system::asset>().lock();
			auto diffuse_asset = assetM->get<asset::image>("paddle");

			auto win = engine->add();
			engine->add<component::window>(win, math::point2i(1280, 1280));

			// stages

			auto stage_2d      = engine->add();
			auto stage_3d      = engine->add();
			auto stage_monitor = engine->add();
			auto stage_chroma  = engine->add();

			engine->add<component::stage>(stage_2d,      "2d");
			engine->add<component::stage>(stage_3d,      "3d");
			engine->add<component::stage>(stage_monitor, "monitor");
			engine->add<component::stage>(stage_chroma,  "chroma");

			// textures

			auto fb_cctv = engine->add();
			auto fb_pong = engine->add();

			engine->add<component::framebuffer>(fb_cctv, win, true, true);

			engine->add<component::framebuffer>(fb_pong, win);
			engine->add<component::color      >(fb_pong, math::point4(0.8, 0.5, 0.3, 0));

			auto tex_box = engine->add();

			engine->add<component::texture>(tex_box, diffuse_asset);

			// models

			auto model_cabinet = engine->add();
			auto model_monitor = engine->add();
			auto model_portal  = engine->add();
			auto model_box     = engine->add();

			engine->add<component::model>(model_cabinet, assetM->get<asset::model>("cabinet"));
			engine->add<component::model>(model_monitor, assetM->get<asset::model>("monitor"));
			engine->add<component::model>(model_portal,  assetM->get<asset::model>("portal"));
			engine->add<component::model>(model_box,     assetM->get<asset::model>("box"));

			// materials

			auto mat_cabinet = engine->add();
			auto mat_monitor = engine->add();
			auto mat_portal  = engine->add();
			auto mat_box     = engine->add();
			auto mat_chroma  = engine->add();

			engine->add<component::material>(mat_cabinet, stage_3d,      tex_box);
			engine->add<component::material>(mat_monitor, stage_monitor, fb_cctv);
			engine->add<component::material>(mat_portal,  stage_monitor, fb_cctv);
			engine->add<component::material>(mat_box,     stage_2d,      tex_box);
			engine->add<component::material>(mat_chroma,  stage_chroma,  fb_cctv);

			// scenes

			auto scene_arcade = engine->add();
			auto scene_pong = engine->add();

			engine->add<component::scene>(scene_arcade);
			engine->add<component::scene>(scene_pong);

			// cameras

			auto camera_arcade = engine->add();
			auto camera_cctv   = engine->add();
			auto camera_pong   = engine->add();

			auto proj_arcade = math::perspective(math::point2(1280, 1280), 0.1, 1000);
			engine->add<component::camera    >(camera_arcade, scene_arcade, win, proj_arcade);
			engine->add<component::position  >(camera_arcade, math::point3(0, 1.2f, 2));
			engine->add<component::renderable>(camera_arcade, scene_arcade, model_cabinet, mat_cabinet);

			auto proj_cctv = math::perspective(math::point2(1280, 1280), 0.1, 1000);
			engine->add<component::camera     >(camera_cctv, scene_arcade, fb_cctv, proj_cctv);
			engine->add<component::position   >(camera_cctv, math::point3(1, 1.6f, 1.5f));
			engine->add<component::orientation>(camera_cctv, math::point3(0, glm::radians(-20.0f), 0));
			engine->add<component::renderable >(camera_cctv, scene_arcade, model_portal, mat_box);

			//engine->add<component::camera>(camera_pong, scene_pong, fb_pong);

			st.keep(camera_arcade);
			st.keep(camera_cctv);
			st.keep(camera_pong);

			// arcade

			const size_t rows = 5;
			const size_t cols = 5;
			for(size_t row = 0; row < rows; ++row) {
				math::decimal z = row * -4.0f;
				for(size_t col = 0; col < cols; ++col) {
					math::decimal x = (math::decimal(col) - cols / 2) * 2;

					auto cabinet = engine->add();
					engine->add<component::position  >(cabinet, math::point3(x, 0, z));
					engine->add<component::renderable>(cabinet, scene_arcade, model_cabinet, mat_cabinet);

					auto viewport_pong = engine->add();
					engine->add<component::position  >(viewport_pong, math::point3(x, 0, z));
					engine->add<component::renderable>(viewport_pong, scene_arcade, model_monitor, mat_monitor);

					/*
					if(row > 0) {
						engine->add<component::orientation>(cabinet)->orient.derivative(0) = math::point3(0, math::PI_OVER<2>, 0);
						engine->add<component::orientation>(viewport_pong)->orient.derivative(0) = math::point3(0, math::PI_OVER<2>, 0);
					}
					*/

					st.keep(cabinet);
					st.keep(viewport_pong);
				}
			}

			// arcade portals

			/*
			auto portal1 = engine->add();
			engine->add<component::position  >(portal1, math::point3(0, 0, -10));
			engine->add<component::scale     >(portal1, math::point3(4, 4, 4));
			engine->add<component::renderable>(portal1, scene_arcade, model_portal, mat_portal);
			st.keep(portal1);
			*/

			// pong

			auto ball = engine->add();
			engine->add<component::position  >(ball)->pos.derivative(0) = math::point3(-0.5, -0.1, 0);
			engine->add<component::scale     >(ball, math::point3(0.02, 0.02, 0));
			engine->add<component::phys      >(ball, detector::box(), responder::rigid());
			engine->add<component::renderable>(ball, scene_pong, model_box, mat_box);
			st.keep(ball);

			auto left_paddle = engine->add();
			engine->add<component::position  >(left_paddle, math::point3(-0.925, 0, 0));
			engine->add<component::scale     >(left_paddle, math::point3(0.025, 0.2, 0));
			engine->add<component::phys      >(left_paddle, detector::box(), responder::stat());
			engine->add<component::renderable>(left_paddle, scene_pong, model_box, mat_box);
			st.keep(left_paddle);

			auto right_paddle = engine->add();
			engine->add<component::position  >(right_paddle, math::point3( 0.925, 0, 0));
			engine->add<component::scale     >(right_paddle, math::point3(0.025, 0.2, 0));
			engine->add<component::phys      >(right_paddle, detector::box(), responder::stat());
			engine->add<component::renderable>(right_paddle, scene_pong, model_box, mat_box);
			st.keep(right_paddle);

			// actions

			namespace kb    = support::action::keyboard;
			namespace mouse = support::action::mouse;

			auto action = engine->get<system::action>().lock();

			struct a_quit_game    : system::action::digital {};

			st.keep(action->bind<a_quit_game>(lifetime::on, [engine](auto) { engine->quit(); }));

			st.keep(action->bind<kb::key<key::Escape>, a_quit_game   >(lifetime::after));
			st.keep(action->bind<kb::key<key::H>>(lifetime::when, [](auto) {
				polar::log()->notice("pong", "hello world");
			}));

			// pong actions

			/*
			struct a_left_paddle  : system::action::analog {};
			struct a_right_paddle : system::action::analog {};

			const math::decimal speed = 1.5;
			auto on_axis_paddle = [engine, speed](core::ref paddle, math::decimal delta) {
				auto position = engine->mutate<component::position>(paddle);
				position->pos.derivative(0)->y = delta * speed;
			};

			st.keep(action->bind<a_left_paddle >(std::bind(on_axis_paddle, left_paddle,  std::placeholders::_2)));
			st.keep(action->bind<a_right_paddle>(std::bind(on_axis_paddle, right_paddle, std::placeholders::_2)));

			st.keep(action->bind<kb::key<key::W>,      a_left_paddle >(lifetime::when,  1));
			st.keep(action->bind<kb::key<key::S>,      a_left_paddle >(lifetime::when, -1));
			st.keep(action->bind<kb::key<key::Up>,     a_right_paddle>(lifetime::when,  1));
			st.keep(action->bind<kb::key<key::Down>,   a_right_paddle>(lifetime::when, -1));
			//st.keep(action->bind<mouse::motion_y, a_left_paddle>());
			*/

			// arcade actions

			struct a_camera_x : system::action::analog {};
			struct a_camera_z : system::action::analog {};

			auto on_camera_x = [engine](auto camera_ref, auto delta) {
				auto pos = engine->mutate<component::position>(camera_ref);
				pos->pos.derivative(0)->x = delta * 4;
			};

			auto on_camera_z = [engine](auto camera_ref, auto delta) {
				auto pos = engine->mutate<component::position>(camera_ref);
				pos->pos.derivative(0)->z = delta * 4;
			};

			st.keep(action->bind<a_camera_x>(on_camera_x));
			st.keep(action->bind<a_camera_z>(on_camera_z));

			st.keep(action->bind<kb::key<key::S>, a_camera_z>(camera_arcade, lifetime::when,  1));
			st.keep(action->bind<kb::key<key::W>, a_camera_z>(camera_arcade, lifetime::when, -1));
			st.keep(action->bind<kb::key<key::D>, a_camera_x>(camera_arcade, lifetime::when,  1));
			st.keep(action->bind<kb::key<key::A>, a_camera_x>(camera_arcade, lifetime::when, -1));

			st.keep(action->bind<kb::key<key::K>, a_camera_z>(camera_cctv, lifetime::when,  1));
			st.keep(action->bind<kb::key<key::I>, a_camera_z>(camera_cctv, lifetime::when, -1));
			st.keep(action->bind<kb::key<key::L>, a_camera_x>(camera_cctv, lifetime::when,  1));
			st.keep(action->bind<kb::key<key::J>, a_camera_x>(camera_cctv, lifetime::when, -1));
		});

		engine.run("root");
	}
} // namespace pong
