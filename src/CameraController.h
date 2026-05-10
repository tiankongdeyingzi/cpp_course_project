#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace godot {

class CameraController : public Camera2D {
    GDCLASS(CameraController, Camera2D)

private:
    Node2D* player1 = nullptr;
    Node2D* player2 = nullptr;
    
    String player1_path = "Player1";
    String player2_path = "Player2";

protected:
    static void _bind_methods();

public:
    CameraController();
    ~CameraController();

    void _ready() override;
    void _process(double delta) override;
    
    // 获取玩家节点引用
    void update_player_references();
    
    // 更新摄像机位置
    void update_camera();
    
    // Getter 和 Setter
    String get_player1_path() const;
    void set_player1_path(const String& p_path);
    
    String get_player2_path() const;
    void set_player2_path(const String& p_path);
};

} // namespace godot

#endif // CAMERA_CONTROLLER_H