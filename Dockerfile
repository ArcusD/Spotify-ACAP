# Use the official Axis ACAP SDK
# For C1210-E use ARCH=armv7hf (ARTPEC-7)
# For I8116-E use ARCH=aarch64 (ARTPEC-8)
ARG ARCH=armv7hf
ARG SDK_VERSION=12.1.0
FROM axisecp/acap-native-sdk:${SDK_VERSION}-${ARCH}

ARG ARCH
ENV ARCH_ENV=${ARCH}

# Install Rust toolchain for cross-compilation
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# Set architecture-specific variables
RUN if [ "$ARCH_ENV" = "aarch64" ]; then \
        rustup target add aarch64-unknown-linux-gnu && \
        echo "RUST_TARGET=aarch64-unknown-linux-gnu" >> /etc/environment && \
        echo "RUST_LINKER=aarch64-linux-gnu-gcc" >> /etc/environment; \
    else \
        rustup target add armv7-unknown-linux-gnueabihf && \
        echo "RUST_TARGET=armv7-unknown-linux-gnueabihf" >> /etc/environment && \
        echo "RUST_LINKER=arm-linux-gnueabihf-gcc" >> /etc/environment; \
    fi

# Install build dependencies
RUN apt-get update && apt-get install -y \
    git \
    pkg-config \
    build-essential \
    libclang-dev \
    dos2unix

# Clone and build librespot
WORKDIR /build
RUN git clone https://github.com/librespot-org/librespot.git
WORKDIR /build/librespot

# Optimize Cargo.toml for minimal binary size
RUN echo '\n[profile.release]\nopt-level = "z"\nlto = true\ncodegen-units = 1\npanic = "abort"\nstrip = true' >> Cargo.toml

# Build librespot with alsa backend and discovery
RUN . /etc/environment && . $HOME/.cargo/env && \
    RUST_TARGET_ENV=$(echo $RUST_TARGET | tr '-' '_') && \
    TARGET_UPPER=$(echo $RUST_TARGET | tr '-' '_' | tr '[:lower:]' '[:upper:]') && \
    SYSROOT=$(find /opt/axis/acapsdk/sysroots -maxdepth 1 -name "${ARCH_ENV}*" | head -n 1) && \
    export CARGO_TARGET_${TARGET_UPPER}_LINKER=$RUST_LINKER && \
    export CC_${RUST_TARGET_ENV}=$RUST_LINKER && \
    export PKG_CONFIG_ALLOW_CROSS=1 && \
    export PKG_CONFIG_PATH=$SYSROOT/usr/lib/pkgconfig && \
    export RUSTFLAGS="-L $SYSROOT/usr/lib -L $SYSROOT/lib -Clink-arg=--sysroot=$SYSROOT" && \
    cargo build --release --target $RUST_TARGET --no-default-features --features "alsa-backend,with-libmdns,rustls-tls-webpki-roots"

# Build the ACAP application
WORKDIR /opt/app
COPY . .
# Copy the built librespot binary
RUN . /etc/environment && \
    cp /build/librespot/target/$RUST_TARGET/release/librespot .

# Update manifest.json with the correct architecture before building

# Fix line endings and make scripts executable
RUN dos2unix event_handler.sh state.cgi log.cgi && \
    chmod +x event_handler.sh state.cgi log.cgi

# Build and package
RUN . /opt/axis/acapsdk/environment-setup* && acap-build . -a librespot -a event_handler.sh -a state.cgi -a log.cgi
