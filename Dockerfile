FROM ubuntu:26.04

ARG RAYLIB_VERSION=6.2
ARG DEBIAN_FRONTEND=noninteractive

# Toolchain + raylib (GLFW desktop backend) build dependencies for X11 and Wayland
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    curl \
    gdb \
    git \
    ninja-build \
    pkg-config \
    libasound2-dev \
    libpulse0 \
    libgl1-mesa-dev libglu1-mesa-dev libegl1-mesa-dev \
    libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev libxext-dev \
    libwayland-dev wayland-protocols libxkbcommon-dev \
    mesa-utils libgl1-mesa-dri \
  && rm -rf /var/lib/apt/lists/*

# Build and install raylib once in the image so the project build doesn't
# have to download/compile it (CMakeLists falls back to FetchContent otherwise)
RUN curl -fsSL "https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz" \
      | tar -xz -C /tmp \
  && cmake -S "/tmp/raylib-${RAYLIB_VERSION}" -B /tmp/raylib-build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_EXAMPLES=OFF \
      -DPLATFORM=Desktop \
  && cmake --build /tmp/raylib-build \
  && cmake --install /tmp/raylib-build \
  && ldconfig \
  && rm -rf /tmp/raylib-*
