#pragma once

// libraries
#include <spimpl.h>

// depthai
#include "depthai/capabilities/ImgFrameCapability.hpp"
#include "depthai/pipeline/DeviceNode.hpp"
#include "depthai/pipeline/datatype/CameraControl.hpp"
#include "depthai/properties/CameraProperties.hpp"
#include "depthai/utility/span.hpp"

namespace dai {
namespace node {

// TODO(before mainline) - API not supported on RVC3 - Camera node needs to be implemented
/**
 * @brief Camera node. Experimental node, for both mono and color types of sensors
 */
class Camera : public DeviceNodeCRTP<DeviceNode, Camera, CameraProperties> {
   public:
    constexpr static const char* NAME = "Camera";
    using DeviceNodeCRTP::DeviceNodeCRTP;
    [[nodiscard]] static std::shared_ptr<Camera> create() {
        auto node = std::make_shared<Camera>();
        node->build();
        return node;
    }
    [[nodiscard]] static std::shared_ptr<Camera> create(std::shared_ptr<Device>& defaultDevice) {
        auto node = std::make_shared<Camera>(defaultDevice);
        node->build();
        return node;
    }
    void build();

   protected:
    Properties& getProperties();
    bool isSourceNode() const override;
    utility::NodeRecordParams getNodeRecordParams() const override;
    Output& getRecordOutput() override;
    Input& getReplayInput() override;

   public:
    /**
     * Constructs Camera node.
     */
    Camera();
    explicit Camera(std::shared_ptr<Device>& defaultDevice);
    explicit Camera(std::unique_ptr<Properties> props);
    /**
     * Computes the scaled size given numerator and denominator
     */
    static int getScaledSize(int input, int num, int denom);

    Node::Output* requestNewOutput(const Capability& capability, bool onHost) override;

    /**
     * Initial control options to apply to sensor
     */
    CameraControl initialControl;

    /**
     * Input for ImageManipConfig message, which can modify crop parameters in runtime
     */
    Input inputConfig{*this, {.name = "inputConfig", .types = {{DatatypeEnum::ImageManipConfig, false}}}};

    /**
     * Input for CameraControl message, which can modify camera parameters in runtime
     *
     */
    Input inputControl{*this, {.name = "inputControl", .types = {{DatatypeEnum::CameraControl, false}}}};

    /**
     * Outputs ImgFrame message that carries NV12 encoded (YUV420, UV plane interleaved) frame data.
     *
     * Suitable for use with VideoEncoder node
     */
    Output video{*this, {.name = "video", .types = {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Outputs ImgFrame message that carries BGR/RGB planar/interleaved encoded frame data.
     *
     * Suitable for use with NeuralNetwork node
     */
    Output preview{*this, {.name = "preview", .types = {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Outputs ImgFrame message that carries NV12 encoded (YUV420, UV plane interleaved) frame data.
     *
     * The message is sent only when a CameraControl message arrives to inputControl with captureStill command set.
     */
    Output still{*this, {.name = "still", .types = {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Outputs ImgFrame message that carries YUV420 planar (I420/IYUV) frame data.
     *
     * Generated by the ISP engine, and the source for the 'video', 'preview' and 'still' outputs
     */
    Output isp{*this, {.name = "isp", .types = {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Outputs ImgFrame message that carries RAW10-packed (MIPI CSI-2 format) frame data.
     *
     * Captured directly from the camera sensor, and the source for the 'isp' output.
     */
    Output raw{*this, {.name = "raw", .types = {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Outputs metadata-only ImgFrame message as an early indicator of an incoming frame.
     *
     * It's sent on the MIPI SoF (start-of-frame) event, just after the exposure of the current frame
     * has finished and before the exposure for next frame starts.
     * Could be used to synchronize various processes with camera capture.
     * Fields populated: camera id, sequence number, timestamp
     */
    Output frameEvent{*this, {.name = "frameEvent", .types = {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Dynamic outputs
     */
    OutputMap dynamicOutputs{*this, "dynamicOutputs", {"", "", {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Input for mocking 'isp' functionality.
     *
     * Default queue is blocking with size 8
     */
    Input mockIsp{*this, {.name = "mockIsp", .queueSize = 8, .types = {{DatatypeEnum::ImgFrame, false}}}};

    /**
     * Specify which board socket to use
     * @param boardSocket Board socket to use
     */
    void setBoardSocket(CameraBoardSocket boardSocket);

    /**
     * Retrieves which board socket to use
     * @returns Board socket to use
     */
    CameraBoardSocket getBoardSocket() const;

    /**
     * Specify which camera to use by name
     * @param name Name of the camera to use
     */
    void setCamera(std::string name);

    /**
     * Retrieves which camera to use by name
     * @returns Name of the camera to use
     */
    std::string getCamera() const;

    /// Set camera image orientation
    void setImageOrientation(CameraImageOrientation imageOrientation);

    /// Get camera image orientation
    CameraImageOrientation getImageOrientation() const;

    // TODO(themarpe) - add back
    // /// Set image type of preview output images.
    // void setPreviewType(ImgFrame::Type type);
    // /// Get image type of preview output frames.
    // ImgFrame::Type getPreviewType() const;
    // /// Set image type of video output images. Supported AUTO, GRAY, YUV420 and NV12.
    // void setVideoType(ImgFrame::Type type);
    // /// Get image type of video output frames. Supported AUTO, GRAY, YUV420 and NV12.
    // ImgFrame::Type getVideoType() const;

    /// Set desired resolution. Sets sensor size to best fit
    void setSize(std::tuple<int, int> size);
    /// Set desired resolution. Sets sensor size to best fit
    void setSize(int width, int height);

    /// Set preview output size
    void setPreviewSize(int width, int height);

    /// Set preview output size, as a tuple <width, height>
    void setPreviewSize(std::tuple<int, int> size);

    /// Set video output size
    void setVideoSize(int width, int height);

    /// Set video output size, as a tuple <width, height>
    void setVideoSize(std::tuple<int, int> size);

    /// Set still output size
    void setStillSize(int width, int height);

    /// Set still output size, as a tuple <width, height>
    void setStillSize(std::tuple<int, int> size);

    // /**
    //  * Set 'isp' output scaling (numerator/denominator), preserving the aspect ratio.
    //  * The fraction numerator/denominator is simplified first to a irreducible form,
    //  * then a set of hardware scaler constraints applies:
    //  * max numerator = 16, max denominator = 63
    //  */
    // void setIspScale(int numerator, int denominator);

    // /// Set 'isp' output scaling, as a tuple <numerator, denominator>
    // void setIspScale(std::tuple<int, int> scale);

    // /**
    //  * Set 'isp' output scaling, per each direction. If the horizontal scaling factor
    //  * (horizNum/horizDen) is different than the vertical scaling factor
    //  * (vertNum/vertDen), a distorted (stretched or squished) image is generated
    //  */
    // void setIspScale(int horizNum, int horizDenom, int vertNum, int vertDenom);

    // /// Set 'isp' output scaling, per each direction, as <numerator, denominator> tuples
    // void setIspScale(std::tuple<int, int> horizScale, std::tuple<int, int> vertScale);

    /**
     * Set rate at which camera should produce frames
     * @param fps Rate in frames per second
     */
    void setFps(float fps);

    /**
     * Isp 3A rate (auto focus, auto exposure, auto white balance, camera controls etc.).
     * Default (0) matches the camera FPS, meaning that 3A is running on each frame.
     * Reducing the rate of 3A reduces the CPU usage on CSS, but also increases the convergence rate of 3A.
     * Note that camera controls will be processed at this rate. E.g. if camera is running at 30 fps, and camera control is sent at every frame,
     * but 3A fps is set to 15, the camera control messages will be processed at 15 fps rate, which will lead to queueing.
     */
    void setIsp3aFps(int isp3aFps);

    /**
     * Get rate at which camera should produce frames
     * @returns Rate in frames per second
     */
    float getFps() const;

    /// Get preview size as tuple
    std::tuple<int, int> getPreviewSize() const;
    /// Get preview width
    int getPreviewWidth() const;
    /// Get preview height
    int getPreviewHeight() const;

    /// Get video size as tuple
    std::tuple<int, int> getVideoSize() const;
    /// Get video width
    int getVideoWidth() const;
    /// Get video height
    int getVideoHeight() const;

    /// Get still size as tuple
    std::tuple<int, int> getStillSize() const;
    /// Get still width
    int getStillWidth() const;
    /// Get still height
    int getStillHeight() const;

    /// Get sensor resolution as size
    std::tuple<int, int> getSize() const;
    /// Get sensor resolution width
    int getWidth() const;
    /// Get sensor resolution height
    int getHeight() const;

    // /// Get 'isp' output resolution as size, after scaling
    // std::tuple<int, int> getIspSize() const;
    // /// Get 'isp' output width
    // int getIspWidth() const;
    // /// Get 'isp' output height
    // int getIspHeight() const;

    // /**
    //  * Specify sensor center crop.
    //  * Resolution size / video size
    //  */
    // void sensorCenterCrop();

    // /**
    //  * Specifies sensor crop rectangle
    //  * @param x Top left X coordinate
    //  * @param y Top left Y coordinate
    //  */
    // void setSensorCrop(float x, float y);

    // /**
    //  * @returns Sensor top left crop coordinates
    //  */
    // std::tuple<float, float> getSensorCrop() const;
    // /// Get sensor top left x crop coordinate
    // float getSensorCropX() const;
    // /// Get sensor top left y crop coordinate
    // float getSensorCropY() const;

    // /**
    //  * Specifies whether preview output should preserve aspect ratio,
    //  * after downscaling from video size or not.
    //  *
    //  * @param keep If true, a larger crop region will be considered to still be able to
    //  * create the final image in the specified aspect ratio. Otherwise video size is resized to fit preview size
    //  */
    // void setPreviewKeepAspectRatio(bool keep);

    // /**
    //  * @see setPreviewKeepAspectRatio
    //  * @returns Preview keep aspect ratio option
    //  */
    // bool getPreviewKeepAspectRatio();

    // /// Get number of frames in preview pool
    // int getPreviewNumFramesPool();
    // /// Get number of frames in video pool
    // int getVideoNumFramesPool();
    // /// Get number of frames in still pool
    // int getStillNumFramesPool();
    // /// Get number of frames in raw pool
    // int getRawNumFramesPool();
    // /// Get number of frames in isp pool
    // int getIspNumFramesPool();

    /// Set the source of the warp mesh or disable
    void setMeshSource(Properties::WarpMeshSource source);
    /// Gets the source of the warp mesh
    Properties::WarpMeshSource getMeshSource() const;

    /**
     * Specify local filesystem paths to the undistort mesh calibration files.
     *
     * When a mesh calibration is set, it overrides the camera intrinsics/extrinsics matrices.
     * Overrides useHomographyRectification behavior.
     * Mesh format: a sequence of (y,x) points as 'float' with coordinates from the input image
     * to be mapped in the output. The mesh can be subsampled, configured by `setMeshStep`.
     *
     * With a 1280x800 resolution and the default (16,16) step, the required mesh size is:
     *
     * width: 1280 / 16 + 1 = 81
     *
     * height: 800 / 16 + 1 = 51
     */
    void loadMeshFile(const dai::Path& warpMesh);

    /**
     * Specify mesh calibration data for undistortion
     * See `loadMeshFiles` for the expected data format
     */
    void loadMeshData(span<const std::uint8_t> warpMesh);

    /**
     * Set the distance between mesh points. Default: (32, 32)
     */
    void setMeshStep(int width, int height);
    /// Gets the distance between mesh points
    std::tuple<int, int> getMeshStep() const;

    /// Set calibration alpha parameter that determines FOV of undistorted frames
    void setCalibrationAlpha(float alpha);
    /// Get calibration alpha parameter that determines FOV of undistorted frames
    std::optional<float> getCalibrationAlpha() const;

    /**
     * Configures whether the camera `raw` frames are saved as MIPI-packed to memory.
     * The packed format is more efficient, consuming less memory on device, and less data
     * to send to host: RAW10: 4 pixels saved on 5 bytes, RAW12: 2 pixels saved on 3 bytes.
     * When packing is disabled (`false`), data is saved lsb-aligned, e.g. a RAW10 pixel
     * will be stored as uint16, on bits 9..0: 0b0000'00pp'pppp'pppp.
     * Default is auto: enabled for standard color/monochrome cameras where ISP can work
     * with both packed/unpacked, but disabled for other cameras like ToF.
     */
    void setRawOutputPacked(bool packed);

   protected:
    void buildStage1() override;

   private:
    class Impl;
    spimpl::impl_ptr<Impl> pimpl;
};

}  // namespace node
}  // namespace dai
