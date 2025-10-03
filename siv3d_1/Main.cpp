# include <Siv3D.hpp> // Siv3D v0.6.16

#include <btBulletDynamicsCommon.h>

#include "PhysicsWorld.h"

void Main() {

	// 物理エンジンの準備
	PhysicsWorld world;

	auto tmp = world.addBox(BoxDesc{s3d::Vec3(100.f, 1.f, 100.f), s3d::Vec3(0, -1, 0), 0.0f });
	tmp->setRestitution(1.0f);

	auto fallingBox = world.addBox(BoxDesc{ s3d::Vec3(1.f, 1.f, 1.f), s3d::Vec3(0, 10, 0), 10.0f });
	fallingBox->setRestitution(0.7f);

	// Background color (remove SRGB curve for a linear workflow)
	const ColorF backgroundColor = ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve();

	// カメラの設定
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
	DebugCamera3D camera{ renderTexture.size(), 30_deg, Vec3{ 10, 16, -32 } };

	const Texture uvChecker{ U"example/texture/uv.png", TextureDesc::MippedSRGB };

	Array<std::unique_ptr<PhysicsObject>> boxes;
	// システムループ
	while (System::Update()) {
		ClearPrint();
		camera.update(2.0);

		// スペースキーでキューブを発射
		if (KeySpace.down())
		{
			// カメラの位置と前方ベクトルを取得
			Vec3 camPos = camera.getEyePosition();
			Vec3 camForward = camera.getLookAtVector();

			// キューブの初期位置（カメラの少し前）
			Vec3 cubePos = camPos + camForward * 2.0;

			// キューブ生成
			auto shotBox = world.addBox(BoxDesc{ Vec3(1.f, 1.f, 1.f), cubePos, 5.0f });
			shotBox->setRestitution(0.7f);

			// 前方へインパルスを加える
			shotBox->applyImpulse(camForward * 100.0); // 30.0は速度調整
			boxes.push_back(std::move(shotBox));
		}

		// worldのステップを進める
		world.step(1.0f / 120.0f);

		// Set up a camera in the current 3D scene
		Graphics3D::SetCameraTransform(camera);

		// [3D rendering]
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backgroundColor) };
			// 位置の取得
			btTransform trans;
			fallingBox->draw();

			auto pos = fallingBox->getRigidBody()->getCenterOfMassTransform().getOrigin();

            // 位置の表示
            Print << U"box position: {:.2F}, {:.2F}, {:.2F}"_fmt(pos.x(), pos.y(), pos.z());

            Plane{0.f, -0.5f, 0.f, 64 }.draw(uvChecker);

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


