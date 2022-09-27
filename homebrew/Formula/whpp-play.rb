class WhppPlay < Formula
  desc "WHPP player based on GStreamer"
  homepage "https://github.com/Eyevinn/whpp-play"
  url "https://github.com/Eyevinn/whpp-play/archive/refs/tags/v0.1.0.tar.gz"
  sha256 "6d321a1c8e8ef10fd650bee5c36d2d863545d4841bdd43f4576f120749954d2a"
  license "Apache-2.0"

  depends_on "cmake" => :build
  depends_on "gstreamer"
  depends_on "gst-plugins-base"
  depends_on "gst-plugins-bad"
  depends_on "gst-libav"
  depends_on "libsoup@2"

  def install
    system "cmake", "-DCMAKE_BUILD_TYPE=Release", "-G", "Unix Makefiles", ".", *std_cmake_args
    system "make", "install"
  end

  test do
    output = shell_output("#{bin}/whpp-play", result = 1).strip
    assert_match "Usage: ", output
  end
end
