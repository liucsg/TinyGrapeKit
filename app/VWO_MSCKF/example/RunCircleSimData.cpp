
#include <glog/logging.h>

#include <TGK/Simulation/VisualWheelCircleSim.h>
#include <TGK/Camera/PinholeRanTanCamera.h>
#include <VWO/ParamLoader.h>
#include <VWO/VWOSystem.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        LOG(ERROR) << "[main]: Please input param_file.";
        return EXIT_FAILURE;
    }

    FLAGS_minloglevel = 0;
    FLAGS_colorlogtostderr = true;
    const std::string param_file = argv[1];

    VWO::Parameter params;
    VWO::LoadParam(param_file, &params);

    // Create a camera.
    const std::shared_ptr<TGK::Camera::Camera> camera = std::make_shared<TGK::Camera::PinholeRadTanCamera>(
        params.cam_intrinsic.width, 
        params.cam_intrinsic.height,
        params.cam_intrinsic.fx, params.cam_intrinsic.fy,
        params.cam_intrinsic.cx, params.cam_intrinsic.cy,
        params.cam_intrinsic.k1, params.cam_intrinsic.k2,
        params.cam_intrinsic.p1, params.cam_intrinsic.p2,
        params.cam_intrinsic.k3);

    // Create the simulator.
    TGK::Simulation::VisualWheelCircleSim sim(camera, 
        params.wheel_param.kl, params.wheel_param.kr, params.wheel_param.b,
        params.extrinsic.O_R_C, params.extrinsic.O_p_C);

    // Create VWO system.
    VWO::VWOSystem vwo_sys(param_file);

    int cnt = 0;
    constexpr int kSkipCnt = 10;

    double timestamp;
    Eigen::Matrix3d G_R_O;
    Eigen::Vector3d G_p_O;
    double left_wheel, right_wheel; 
    std::vector<Eigen::Vector2d> features;
    std::vector<long int> feature_ids;
    cv::Mat image;
    while (sim.SimOneFrame(&timestamp, &G_R_O, &G_p_O, &left_wheel, &right_wheel, &features, &feature_ids, &image)) {
        // const Eigen::Vector2d rand_wheel = Eigen::Vector2d::Random() * 50.;
        vwo_sys.FeedWheelData(timestamp, left_wheel, right_wheel);
        
        if (++cnt <= kSkipCnt) { continue; }
        cnt = 0;
        vwo_sys.FeedSimData(timestamp, image, features, feature_ids);
    }
    
    std::cin.ignore();

    return EXIT_SUCCESS;
}