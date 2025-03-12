FROM gcc:13.2 AS build

SHELL ["/bin/bash", "-c"]

WORKDIR /app

# Install development packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    libsdl2-dev \
    libsdl2-gfx-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-net-dev \
    libsdl2-ttf-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Create a runtime directory and set XDG_RUNTIME_DIR
RUN mkdir -p /tmp/runtime && chmod 0700 /tmp/runtime
ENV XDG_RUNTIME_DIR=/tmp/runtime

COPY . /app

RUN ./launch.sh

CMD ["./pikselite"]
