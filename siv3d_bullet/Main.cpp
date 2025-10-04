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
    constexpr s3d::Vec3 CameraInitialPosition{5, 15, -20};
    constexpr s3d::Vec3 CameraInitialLookAt{5, 0, 10};
    constexpr double CameraFov = 30_deg;

    // Cube settings
    constexpr s3d::Vec3 CubeSize{1.0, 1.0, 1.0};
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

    auto floor = world.createBox(BoxDesc{s3d::Vec3(WallLength, WallThickness, WallLength),
                                         s3d::Vec3(WallLength / 2, -WallThickness / 2, WallLength / 2), 0.0f});
    floor->setRestitution(WallRestitution);
    floor->setColor(ColorF{0.15, 0.15, 0.15}); // 床：明るいグレー

    auto wall_l = world.createBox(BoxDesc{s3d::Vec3(WallThickness, WallLength, WallLength),
                                          s3d::Vec3(-WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f});
    wall_l->setRestitution(WallRestitution);
    wall_l->setColor(ColorF{0.3, 0.7, 0.4}); // 左壁：緑系

    auto wall_r =
        world.createBox(BoxDesc{s3d::Vec3(WallThickness, WallLength, WallLength),
                                s3d::Vec3(WallLength + WallThickness / 2, WallLength / 2, WallLength / 2), 0.0f});
    wall_r->setRestitution(WallRestitution);
    wall_r->setColor(ColorF{0.4, 0.5, 0.9}); // 右壁：青系

    auto wall_b =
        world.createBox(BoxDesc{s3d::Vec3(WallLength, WallLength, WallThickness),
                                s3d::Vec3(WallLength / 2, WallLength / 2, WallLength + WallThickness / 2), 0.0f});
    wall_b->setRestitution(WallRestitution);
    wall_b->setColor(ColorF{0.9, 0.5, 0.4}); // 奥壁：赤系

    // Background color (remove SRGB curve for a linear workflow)
    const ColorF backgroundColor = ColorF{0.4, 0.6, 0.8}.removeSRGBCurve();

    // カメラの設定
    const MSRenderTexture renderTexture{Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes};
    DebugCamera3D camera{renderTexture.size(), CameraFov, CameraInitialPosition, CameraInitialLookAt};

    const Texture uvChecker{U"example/texture/uv.png", TextureDesc::MippedSRGB};

    Array<std::unique_ptr<PhysicsObject>> boxes;
    // システムループ
    while (System::Update())
    {
        ClearPrint();
        Print << U"cube num:{}"_fmt(boxes.size());
        camera.update(CameraSpeed);

        // スペースキーでキューブを発射
        if (KeySpace.down())
        {
            // カメラの位置と前方ベクトルを取得
            Vec3 camPos = camera.getEyePosition();
            Vec3 camForward = camera.getLookAtVector();

            // キューブの初期位置（カメラの少し前）
            Vec3 cubePos = camPos + camForward * 20.0;

            // キューブ生成
            auto shotBox = world.createBox(BoxDesc{CubeSize, cubePos, CubeMass});
            shotBox->setRestitution(CubeRestitution);

            // 前方へインパルスを加える
            shotBox->applyImpulse(camForward * CubeLaunchImpulse); // 30.0は速度調整
            boxes.push_back(std::move(shotBox));
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
            auto shotSphere = world.createSphere(SphereDesc{SphereRadius, spherePos, SphereMass});
            shotSphere->setRestitution(SphereRestitution);
            // 前方へインパルスを加える
            shotSphere->applyImpulse(camForward * SphereLaunchImpulse); // 20.0は速度調整
            boxes.push_back(std::move(shotSphere));
        }

        // worldのステップを進める
        world.step(Scene::DeltaTime());

        // Set up a camera in the current 3D scene
        Graphics3D::SetCameraTransform(camera);

        // [3D rendering]
        {
            const ScopedRenderTarget3D target{renderTexture.clear(backgroundColor)};

            // 位置の表示
            // Print << U"box position: {:.2F}, {:.2F}, {:.2F}"_fmt(pos.x(), pos.y(), pos.z());

            floor->draw();
            wall_b->draw();
            wall_l->draw();
            wall_r->draw();

            for (auto& box : boxes)
            {
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
