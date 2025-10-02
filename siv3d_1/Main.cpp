# include <Siv3D.hpp> // Siv3D v0.6.16

#include <btBulletDynamicsCommon.h>

#include "PhysicsWorld.h"

void Main() {

	// 物理エンジンの準備
	PhysicsWorld world;

	btRigidBody* tmp = world.addBox(btVector3(50.0f, 0.5f, 50.0f), btVector3(0, -1, 0), 0.0f);
	tmp->setRestitution(1.0f);

	btRigidBody* fallingBox = world.addBox(btVector3(0.5f, 0.5f, 0.5f), btVector3(0, 10, 0), 10.0f);
	fallingBox->setRestitution(0.7f);

	// Background color (remove SRGB curve for a linear workflow)
	const ColorF backgroundColor = ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve();

	// カメラの設定
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
	DebugCamera3D camera{ renderTexture.size(), 30_deg, Vec3{ 10, 16, -32 } };

	const Texture uvChecker{ U"example/texture/uv.png", TextureDesc::MippedSRGB };


	// システムループ
	while (System::Update()) {
		ClearPrint();
		camera.update(2.0);

		// worldのステップを進める
		world.step(1.0f / 120.0f);

		// Set up a camera in the current 3D scene
		Graphics3D::SetCameraTransform(camera);

		// [3D rendering]
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backgroundColor) };
			// 位置の取得
			btTransform trans;
			fallingBox->getMotionState()->getWorldTransform(trans);
			btVector3 pos = trans.getOrigin();

			// BulletのbtTransformをSiv3DのMat4に変換
			const btVector3& origin = trans.getOrigin();
			const btQuaternion& rotation = trans.getRotation();

			// Siv3DのVec3とQuaternionに変換
			const Vec3 position(origin.getX(), origin.getY(), origin.getZ());
			const Quaternion quat(rotation.getX(), rotation.getY(), rotation.getZ(), rotation.getW());

			// 描画用のMat4を作成 (位置と回転を適用)
			const Mat4x4 drawMatrix = Mat4x4::Translate(position) * Mat4x4::Rotate(quat);

			const Vec3 boxSize(1.0f, 1.0f, 1.0f);
			Box(boxSize).draw(drawMatrix, Palette::Orange);

			// 位置の表示
			Print << U"box position: {:.2F}, {:.2F}, {:.2F}"_fmt(pos.x(), pos.y(), pos.z());

			Plane{0.f, -0.5f, 0.f, 64 }.draw(uvChecker);
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


