#include <Siv3D.hpp> // Siv3D v0.6.16

#include <btBulletDynamicsCommon.h>

#include "PhysicsWorld.h"

namespace
{
	// World settings
	constexpr double WallLength = 10.0;
	constexpr double WallThickness = 1.0;
	constexpr float WallRestitution = 1.0f;

	// Camera settings
	constexpr double CameraSpeed = 20.0;
	constexpr s3d::Vec3 CameraInitialPosition{ 5, 15, -20 };
	constexpr s3d::Vec3 CameraInitialLookAt{ 5, 0, 10 };
	constexpr double CameraFov = 30_deg;

	// Cube settings
	constexpr s3d::Vec3 CubeSize{ 1.0, 1.0, 1.0 };
	constexpr float CubeMass = 5.0f;
	constexpr float CubeRestitution = 0.7f;
	constexpr double CubeLaunchImpulse = 30.0;

	// Sphere settings
	constexpr float SphereRadius = 0.5f;
	constexpr float SphereMass = 5.0f;
	constexpr float SphereRestitution = 0.7f;
	constexpr double SphereLaunchImpulse = 20.0;
} // namespace

void Main()
{

	// 物理エンジンの準備
	PhysicsWorld world;

	Array<std::unique_ptr<PhysicsObject>> physicsObjects;

	auto floor = world.createBox(BoxDesc{ s3d::Vec3(WallLength, WallThickness, WallLength),
										 s3d::Vec3(WallLength / 2, -WallThickness / 2, WallLength / 2), 0.0f });
	floor->setRestitution(WallRestitution);
	floor->setColor(s3d::Linear::Palette::Silver);

	auto wall_l = world.createBox(BoxDesc{ s3d::Vec3(WallThickness, WallLength, WallLength),
										  s3d::Vec3(-WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f });
	wall_l->setRestitution(WallRestitution);
	wall_l->setColor(s3d::Linear::Palette::Powderblue);

	auto wall_r =
		world.createBox(BoxDesc{ s3d::Vec3(WallThickness, WallLength, WallLength),
								s3d::Vec3(WallLength + WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f });
	wall_r->setRestitution(WallRestitution);
	wall_r->setColor(s3d::Linear::Palette::Powderblue);

	auto wall_b =
		world.createBox(BoxDesc{ s3d::Vec3(WallLength, WallLength, WallThickness),
								s3d::Vec3(WallLength / 2, WallLength / 2, WallLength + WallThickness / 2), 0.0f });
	wall_b->setRestitution(WallRestitution);
	wall_b->setColor(s3d::Linear::Palette::Powderblue);

	physicsObjects.push_back(std::move(floor));
	physicsObjects.push_back(std::move(wall_l));
	physicsObjects.push_back(std::move(wall_r));
	physicsObjects.push_back(std::move(wall_b));

	// Background color (remove SRGB curve for a linear workflow)
	const ColorF backgroundColor = ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve();

	// カメラの設定
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
	DebugCamera3D camera{ renderTexture.size(), CameraFov, CameraInitialPosition, CameraInitialLookAt };

	const Texture uvChecker{ U"example/texture/uv.png", TextureDesc::MippedSRGB };

	// 音声ファイルの読み込み
	// 効果音ラボから音源は取得
	Audio cubeShootSound{ U"example/sounds/shoot.mp3" };
	Audio sphereShootSound{ U"example/sounds/shoot.mp3" };

	// 音量の初期値（0.0 ～ 1.0）
	double volume = 0.5;

	// システムループ
	while (System::Update())
	{
		ClearPrint();
		Print << U"Object num:{}"_fmt(physicsObjects.size());
		Print << Profiler::FPS();
		camera.update(CameraSpeed);
		for (auto& object : physicsObjects)
		{
			object->update();
		}

		// スペースキーでキューブを発射
		if (KeySpace.down())
		{
			// カメラの位置と前方ベクトルを取得
			Vec3 camPos = camera.getEyePosition();
			Vec3 camForward = camera.getLookAtVector();

			// キューブの初期位置（カメラの少し前）
			Vec3 cubePos = camPos + camForward * 20.0;

			// キューブ生成
			auto shotBox = world.createBox(BoxDesc{ CubeSize, cubePos, CubeMass });
			shotBox->setRestitution(CubeRestitution);
			shotBox->setColor(s3d::Linear::Palette::Gainsboro);

			// 前方へインパルスを加える
			shotBox->applyImpulse(camForward * CubeLaunchImpulse);
			physicsObjects.push_back(std::move(shotBox));

			// キューブ発射音を再生
			// Play()は重複再生しないため、
			// playOneShot()で多重再生する
			cubeShootSound.playOneShot();
		}

		// oキーでsphereを発射
		if (KeyO.down())
		{
			// カメラの位置と前方ベクトルを取得
			Vec3 camPos = camera.getEyePosition();
			Vec3 camForward = camera.getLookAtVector();
			// 球の初期位置（カメラの少し前）
			Vec3 spherePos = camPos + camForward * 20.0;
			// 球生成
			auto shotSphere = world.createSphere(SphereDesc{ SphereRadius, spherePos, SphereMass });
			shotSphere->setRestitution(SphereRestitution);
			shotSphere->setColor(s3d::Linear::Palette::Lightsteelblue);

			// 前方へインパルスを加える
			shotSphere->applyImpulse(camForward * SphereLaunchImpulse); // 20.0は速度調整
			physicsObjects.push_back(std::move(shotSphere));

			// 球発射音を再生
			sphereShootSound.playOneShot();
		}

		// worldのステップを進める
		world.step(Scene::DeltaTime());

		// 座標が一定以下ならオブジェクトを削除
		physicsObjects.remove_if([](const std::unique_ptr<PhysicsObject>& obj)
								 { return obj->getPosition().y < -10.0; });

		// Set up a camera in the current 3D scene
		Graphics3D::SetCameraTransform(camera);

		// [3D rendering]
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backgroundColor) };

			// for debug
			// Plane{64}.draw(uvChecker);
			for (auto& object : physicsObjects)
			{
				object->draw();
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
