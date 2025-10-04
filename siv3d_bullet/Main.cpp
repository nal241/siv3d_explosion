# include <Siv3D.hpp> // Siv3D v0.6.16

#include <btBulletDynamicsCommon.h>

#include "PhysicsWorld.h"

void Main() {

	// 物理エンジンの準備
	PhysicsWorld world;

	double wallLength = 10.0;
	double wallThickness = 1.0;

	auto floor = world.addBox(BoxDesc{ s3d::Vec3(wallLength, wallThickness, wallLength), s3d::Vec3(wallLength / 2, -wallThickness / 2, wallLength / 2), 0.0f });
	floor->setRestitution(1.0f);
	floor->setColor(ColorF{ 0.15, 0.15, 0.15 }); // 床：明るいグレー

	auto wall_l = world.addBox(BoxDesc{ s3d::Vec3(wallThickness, wallLength, wallLength), s3d::Vec3(-wallThickness / 2, wallLength / 2, wallLength / 2), 0.0f });
	wall_l->setRestitution(1.0f);
	wall_l->setColor(ColorF{ 0.3, 0.7, 0.4 }); // 左壁：緑系

	auto wall_r = world.addBox(BoxDesc{ s3d::Vec3(wallThickness, wallLength, wallLength), s3d::Vec3(wallLength + wallThickness / 2, wallLength / 2, wallLength / 2), 0.0f });
	wall_r->setRestitution(1.0f);
	wall_r->setColor(ColorF{ 0.4, 0.5, 0.9 }); // 右壁：青系

	auto wall_b = world.addBox(BoxDesc{ s3d::Vec3(wallLength, wallLength, wallThickness), s3d::Vec3(wallLength / 2, wallLength / 2, wallLength + wallThickness / 2), 0.0f });
	wall_b->setRestitution(1.0f);
	wall_b->setColor(ColorF{ 0.9, 0.5, 0.4 }); // 奥壁：赤系


	// Background color (remove SRGB curve for a linear workflow)
	const ColorF backgroundColor = ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve();

	// カメラの設定
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
	DebugCamera3D camera{ renderTexture.size(), 30_deg, Vec3{ 5, 15, -20 }, Vec3{5,0,10} };

	const Texture uvChecker{ U"example/texture/uv.png", TextureDesc::MippedSRGB };

	Array<std::unique_ptr<PhysicsObject>> boxes;
	// システムループ
	while (System::Update()) {
		ClearPrint();
		Print << U"cube num:{}"_fmt(boxes.size());
		camera.update(20.0);

		// スペースキーでキューブを発射
		if (KeySpace.down())
		{
			// カメラの位置と前方ベクトルを取得
			Vec3 camPos = camera.getEyePosition();
			Vec3 camForward = camera.getLookAtVector();

			// キューブの初期位置（カメラの少し前）
			Vec3 cubePos = camPos + camForward * 20.0;

			// キューブ生成
			auto shotBox = world.addBox(BoxDesc{ Vec3(1.f, 1.f, 1.f), cubePos, 5.0f });
			shotBox->setRestitution(0.7f);

			// 前方へインパルスを加える
			shotBox->applyImpulse(camForward * 30.0); // 30.0は速度調整
			boxes.push_back(std::move(shotBox));
		}

		// worldのステップを進める
		world.step(Scene::DeltaTime());

		// Set up a camera in the current 3D scene
		Graphics3D::SetCameraTransform(camera);

		// [3D rendering]
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backgroundColor) };

            // 位置の表示
            //Print << U"box position: {:.2F}, {:.2F}, {:.2F}"_fmt(pos.x(), pos.y(), pos.z());

			floor->draw();
			wall_b->draw();
			wall_l->draw();
			wall_r->draw();

			for (auto& box : boxes) {
				box->draw();
			}
        }

        // [2D rendering]
        {
            // Flush 3D rendering commands before multisample resolve
            Graphics3D::Flush();

            // Multisample resolve
            renderTexture.resolve();

            // Transfer renderTexture to the current 2D scene (default scene)
            Shader::LinearToScreen(renderTexture);
        }
    }
}


