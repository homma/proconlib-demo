#include <SDL3/SDL.h>
#include <proconlib/procon_manager.hpp>

#include <cstdlib>
// needs to include after cstdlib
#include <GamepadMotionHelpers/GamepadMotion.hpp>

#include <raylib.h>
#include <raymath.h>

#include <format>
#include <memory>
#include <numbers>
#include <print>

struct Procon {
  ProconManager procon;
  GamepadMotion motion;

  auto scan() -> bool;
  auto update() -> bool;

  auto get_quaternion() -> std::tuple<float, float, float, float>;

  auto print_procon() -> void;
  auto print_motion() -> void;
  auto print() -> void;
};

auto Procon::scan() -> bool {
  auto exists = this->procon.scan();

  if (exists) {
    this->motion.SetCalibrationMode(
        GamepadMotionHelpers::CalibrationMode::Stillness |
        GamepadMotionHelpers::CalibrationMode::SensorFusion);
  }

  return exists;
}

auto Procon::update() -> bool {
  if (!this->procon.connected()) {
    if (!procon.scan()) {
      return false;
    }
  }

  auto updated = this->procon.update();

  if (updated) {

    // For gyro, SDL reports numbers in radian.
    // So, we convert radian to degree.
    //
    // see:
    // https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_switch.c#L1080-L1083
    //
    float r2d = 180.0 / std::numbers::pi;

    // note for coordinates:
    //
    // GamepadMotion uses playstation controller coordinates (Y-up).
    //
    // SDL converts procon coordinates into playstation controller
    // coordinates inside its implementation.
    // So, we do not need to take care of the difference.
    //
    // see:
    // https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_switch.c#L2291-L2294

    this->motion.ProcessMotion(this->procon.gyro[0] * r2d, //
                               this->procon.gyro[1] * r2d, //
                               this->procon.gyro[2] * r2d, //
                               this->procon.accel[0],      //
                               this->procon.accel[1],      //
                               this->procon.accel[2],      //
                               this->procon.delta_time / 1000.0);
  }

  return updated;
}

auto Procon::get_quaternion() -> std::tuple<float, float, float, float> {
  float w, x, y, z;

  this->motion.GetOrientation(w, x, y, z);

  return std::tuple<float, float, float, float>{w, x, y, z};
}

auto Procon::print_procon() -> void { this->procon.print_data(); }

auto Procon::print_motion() -> void {
  auto [w, x, y, z] = this->get_quaternion();

  std::println("w: {}, x: {}, y: {}, z: {}", w, x, y, z);
}

auto Procon::print() -> void {
  this->print_procon();
  this->print_motion();
}

//------------------------------------------------------------------------------

auto update_screen(Procon &procon, Model &model) -> void {

  // Color
  auto white = Color{255, 255, 255, 255};
  auto black = Color{0, 0, 0, 255};

  ClearBackground(white);

  // Camera
  auto camera = Camera();
  // look from behind
  camera.position = Vector3{0.0, 50.0, -120.0};
  camera.target = Vector3{0.0, 10.0, 0.0};
  camera.up = Vector3{0.0, 1.0, 0.0};
  camera.fovy = 30.0;
  camera.projection = CAMERA_PERSPECTIVE;

  // Model transformation
  auto [w, x, y, z] = procon.get_quaternion();

  // axis and angle
  Vector3 axis;
  float angle;
  auto q = Quaternion{x, y, z, w};
  QuaternionToAxisAngle(q, &axis, &angle);

  // radian to degree
  float r2d = 180.0 / std::numbers::pi;
  auto deg_axis = Vector3{axis.x * r2d, axis.y * r2d, axis.z * r2d};
  auto deg_angle = angle * r2d;

  Vector3 position = {0.0, 0.0, 0.0};
  Vector3 scale = {1.0, 1.0, 1.0};

  BeginDrawing();

  BeginMode3D(camera);

  DrawModelWiresEx(model, position, deg_axis, deg_angle, scale, black);
  // another method to transform a model: matrix transformation
  //
  // model.transform = QuaternionToMatrix(q);
  // DrawModelWires(model, position, 1.0, black);

  EndMode3D();

  // display motion values on the screen
  auto left = 10;
  auto height = 30;

  {
    auto str = std::format("w: {}", w);
    auto text = str.c_str();
    auto top = 10;

    DrawText(text, left, top, height, black);
  }
  {
    auto str = std::format("x: {}", x);
    auto text = str.c_str();
    auto top = 50;

    DrawText(text, left, top, height, black);
  }
  {
    auto str = std::format("y: {}", y);
    auto text = str.c_str();
    auto top = 90;

    DrawText(text, left, top, height, black);
  }
  {
    auto str = std::format("z: {}", z);
    auto text = str.c_str();
    auto top = 130;

    DrawText(text, left, top, height, black);
  }
  {
    auto str = std::format("ax: {}", deg_axis.x);
    auto text = str.c_str();
    auto top = 170;

    DrawText(text, left, top, height, black);
  }
  {
    auto str = std::format("ay: {}", deg_axis.y);
    auto text = str.c_str();
    auto top = 210;

    DrawText(text, left, top, height, black);
  }
  {
    auto str = std::format("az: {}", deg_axis.z);
    auto text = str.c_str();
    auto top = 250;

    DrawText(text, left, top, height, black);
  }
  {
    auto str = std::format("angle: {}", deg_angle);
    auto text = str.c_str();
    auto top = 290;

    DrawText(text, left, top, height, black);
  }

  EndDrawing();
}

auto update(Procon &procon, Model &model) -> void {
  procon.update();
  update_screen(procon, model);
}

auto loop(Procon &procon) -> void {
  // create a 3D model
  auto mesh = GenMeshCone(10.0, 20.0, 10.0);
  auto model = LoadModelFromMesh(mesh);

  while (!WindowShouldClose()) {
    update(procon, model);
  }

  // clean up
  UnloadModel(model);
}

auto init_sdl() -> void {
  if (!SDL_Init(SDL_INIT_GAMEPAD)) {
    exit(1);
  }
}

auto init_procon() -> Procon {
  auto procon = Procon();

  if (!procon.scan()) {
    exit(1);
  }

  return procon;
}

auto create_window() -> void {
  int width = 1000;
  int height = 600;
  auto title = "My Procon Window";
  InitWindow(width, height, title);
}

auto main() -> int {
  init_sdl();

  auto procon = init_procon();

  create_window();

  loop(procon);

  CloseWindow();
}
