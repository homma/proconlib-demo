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

  auto is_button_pressed(SDL_GamepadButton button) -> bool;
  auto is_zbutton_pressed(SDL_GamepadAxis axis) -> bool;
  auto get_axis(SDL_GamepadAxis axis) -> int16_t;

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

auto Procon::is_button_pressed(SDL_GamepadButton button) -> bool {
  return this->procon.is_button_pressed(button);
}

auto Procon::is_zbutton_pressed(SDL_GamepadAxis axis) -> bool {
  return this->procon.is_zbutton_pressed(axis);
}

auto Procon::get_axis(SDL_GamepadAxis axis) -> int16_t {
  return this->procon.get_axis(axis);
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

auto draw_gamepad(Procon &procon) -> void {

  // Color
  auto white = Color{255, 255, 255, 255};
  auto black = Color{0, 0, 0, 255};

  // quaternion data
  auto [w, x, y, z] = procon.get_quaternion();

  auto bt_up = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_DPAD_UP);
  auto bt_down = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_DPAD_DOWN);
  auto bt_left = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_DPAD_LEFT);
  auto bt_right = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
  auto bt_a = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_EAST);
  auto bt_b = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_SOUTH);
  auto bt_x = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_NORTH);
  auto bt_y = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_WEST);
  auto bt_l = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
  auto bt_r = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
  auto bt_l_stick = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_LEFT_STICK);
  auto bt_r_stick = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_RIGHT_STICK);
  auto bt_plus = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_START);
  auto bt_minus = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_BACK);
  auto bt_home = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_GUIDE);
  auto bt_capture = procon.is_button_pressed(SDL_GAMEPAD_BUTTON_MISC1);

  auto bt_zl = procon.is_zbutton_pressed(SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
  auto bt_zr = procon.is_zbutton_pressed(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);

  auto st_l_x = procon.get_axis(SDL_GAMEPAD_AXIS_LEFTX);
  auto st_l_y = procon.get_axis(SDL_GAMEPAD_AXIS_LEFTY);
  auto st_r_x = procon.get_axis(SDL_GAMEPAD_AXIS_RIGHTX);
  auto st_r_y = procon.get_axis(SDL_GAMEPAD_AXIS_RIGHTY);

  // display quaternion value
  auto font_size = 20;

  {
    auto str = std::format("w: {}", w);
    auto text = str.c_str();
    auto left = 10;
    auto top = 10;

    DrawText(text, left, top, font_size, black);
  }
  {
    auto str = std::format("x: {}", x);
    auto text = str.c_str();
    auto left = 10;
    auto top = 40;

    DrawText(text, left, top, font_size, black);
  }
  {
    auto str = std::format("y: {}", y);
    auto text = str.c_str();
    auto left = 10;
    auto top = 70;

    DrawText(text, left, top, font_size, black);
  }
  {
    auto str = std::format("z: {}", z);
    auto text = str.c_str();
    auto left = 10;
    auto top = 100;

    DrawText(text, left, top, font_size, black);
  }

  {
    auto str = std::format("[-]: {}", bt_minus);
    auto text = str.c_str();
    auto left = 10;
    auto top = 600 - 320;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("Capt.: {}", bt_capture);
    auto text = str.c_str();
    auto left = 170;
    auto top = 600 - 320;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("L: {}", bt_l);
    auto text = str.c_str();
    auto left = 10;
    auto top = 600 - 280;
    auto width = 300;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("ZL: {}", bt_zl);
    auto text = str.c_str();
    auto left = 10;
    auto top = 600 - 240;
    auto width = 300;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("Up: {}", bt_up);
    auto text = str.c_str();
    auto left = 80;
    auto top = 600 - 200;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("Left: {}", bt_left);
    auto text = str.c_str();
    auto left = 10;
    auto top = 600 - 160;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("Right: {}", bt_right);
    auto text = str.c_str();
    auto left = 170;
    auto top = 600 - 160;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("Down: {}", bt_down);
    auto text = str.c_str();
    auto left = 80;
    auto top = 600 - 120;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("LS: {}", bt_l_stick);
    auto text = str.c_str();
    auto left = 10;
    auto top = 600 - 80;
    auto width = 140;
    auto height = 70;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 25;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("LS X: {}", st_l_x);
    auto text = str.c_str();
    auto left = 170;
    auto top = 600 - 80;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 5;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("LS Y: {}", st_l_y);
    auto text = str.c_str();
    auto left = 170;
    auto top = 600 - 40;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 5;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("Home: {}", bt_home);
    auto text = str.c_str();
    auto left = 680 + 10;
    auto top = 600 - 320;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("[+]: {}", bt_plus);
    auto text = str.c_str();
    auto left = 680 + 170;
    auto top = 600 - 320;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("R: {}", bt_r);
    auto text = str.c_str();
    auto left = 680 + 10;
    auto top = 600 - 280;
    auto width = 300;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("ZR: {}", bt_zr);
    auto text = str.c_str();
    auto left = 680 + 10;
    auto top = 600 - 240;
    auto width = 300;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("X: {}", bt_x);
    auto text = str.c_str();
    auto left = 680 + 80;
    auto top = 600 - 200;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("Y: {}", bt_y);
    auto text = str.c_str();
    auto left = 680 + 10;
    auto top = 600 - 160;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("A: {}", bt_a);
    auto text = str.c_str();
    auto left = 680 + 170;
    auto top = 600 - 160;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("B: {}", bt_b);
    auto text = str.c_str();
    auto left = 680 + 80;
    auto top = 600 - 120;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("RS: {}", bt_r_stick);
    auto text = str.c_str();
    auto left = 680 + 10;
    auto top = 600 - 80;
    auto width = 140;
    auto height = 70;
    auto color = black;
    auto tx_left = left + 10;
    auto tx_top = top + 25;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("LS R: {}", st_r_x);
    auto text = str.c_str();
    auto left = 680 + 170;
    auto top = 600 - 80;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 5;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }

  {
    auto str = std::format("LS R: {}", st_r_y);
    auto text = str.c_str();
    auto left = 680 + 170;
    auto top = 600 - 40;
    auto width = 140;
    auto height = 30;
    auto color = black;
    auto tx_left = left + 5;
    auto tx_top = top + 5;

    DrawRectangleLines(left, top, width, height, color);
    DrawText(text, tx_left, tx_top, font_size, color);
  }
}

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

  draw_gamepad(procon);

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
