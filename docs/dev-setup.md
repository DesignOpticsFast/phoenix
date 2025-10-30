Developer Environment Setup — Amazon Linux 2023 + Qt 6.10.0

This guide provides complete steps for setting up a development environment for Phoenix/Bedrock/Rosetta. It covers installing prerequisites, downloading Qt from S3, verifying integrity, configuring environment variables, and running a smoke test build.

> ⚠️ **Updated for Phase 0.5 Gate PASS (2025-10-18): Qt 6.10.0 + Qt Graphs / gRPC UDS foundation.**  
> 🚀 **NEW (2025-01-21): Tailscale integration for secure development workflow.**

---

## 🎯 **Development Policy**

### **ALL CODE CHANGES MUST BE DONE ON DEV-01 FIRST**

**Rationale:**

- ✅ **Consistent build environment** - Linux-based development
- ✅ **Automated testing** - CI/CD pipelines run on dev-01
- ✅ **Resource debugging** - Can test GUI with Xvfb
- ✅ **Version control** - All changes tracked in Git
- ✅ **Collaboration** - Team can access and review changes
- ✅ **Quality assurance** - Automated tests and validation

**Workflow:**

1. **Develop on dev-01** - Make all code changes
2. **Test on dev-01** - Build and test with Xvfb
3. **Commit from dev-01** - Push changes to repository
4. **Test locally** - Pull changes and test on local machine

⸻

## 🔗 **Tailscale Integration**

### **Connection Setup**

```bash
# Install Tailscale on dev-01 (already configured)
# Install Tailscale on local machine
# Join same Tailscale network

# Connect via Tailscale IP
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75
```

### **Benefits of Tailscale**

- ✅ **Direct connection** - No NAT issues
- ✅ **Secure** - Encrypted tunnel
- ✅ **Reliable** - No port forwarding needed
- ✅ **Fast** - Low latency connection
- ✅ **Persistent** - Always available

### **Development Commands**

```bash
# Connect to dev-01
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75
cd /home/ec2-user/workspace/phoenix

# Test GUI with Xvfb
xvfb-run -a ./phoenix_app

# Capture screenshots
xvfb-run -a ./phoenix_app &
sleep 5
import -window root screenshot.png
```

⸻

0) System & Toolchain

Update the system and install required packages:

sudo dnf update -y
sudo dnf install -y gcc gcc-c++ cmake ninja-build git \
  mesa-libGL-devel libglvnd-devel unzip xz tar

If AWS CLI v2 is missing:

cd /tmp
curl -fsSL -o awscliv2.zip <https://awscli.amazonaws.com/awscli-exe-linux-x86_64.zip>
unzip awscliv2.zip
sudo ./aws/install --update
aws --version

⸻

1) Download Qt tarball + SHA256 from S3

Artifacts are versioned and stored with a checksum file.

aws s3 cp s3://phoenix-dev-artifacts-us-east-2-mn/artifacts/6.10.0/Qt-6.10.0-gcc_64.tgz /tmp/
aws s3 cp s3://phoenix-dev-artifacts-us-east-2-mn/artifacts/6.10.0/Qt-6.10.0-gcc_64.sha256 /tmp/

⸻

2) Verify SHA256 checksum

cd /tmp
sha256sum -c Qt-6.9.3-gcc_64.sha256

Expected output:

Qt-6.9.3-gcc_64.tgz: OK

If it does not match, re-download and re-verify.

⸻

3) Extract to /opt/Qt

sudo mkdir -p /opt/Qt
sudo tar -xzf /tmp/Qt-6.9.3-gcc_64.tgz -C /opt
ls -l /opt/Qt/6.9.3/gcc_64/bin/qmake

Ensure that qmake exists in the extracted directory.

⸻

4) Set environment variables globally

Append the environment variables globally so all users/shells inherit them.

echo 'PATH=/opt/Qt/6.9.3/gcc_64/bin:$PATH' | sudo tee -a /etc/environment
echo 'CMAKE_PREFIX_PATH=/opt/Qt/6.9.3/gcc_64/lib/cmake' | sudo tee -a /etc/environment
echo 'ARTIFACTS_BUCKET=phoenix-dev-artifacts-us-east-2-mn' | sudo tee -a /etc/environment

Open a new shell to apply changes:

exec bash -l

Verify:

which qmake
qmake -v
echo $CMAKE_PREFIX_PATH

Expected:
 • which qmake → /opt/Qt/6.9.3/gcc_64/bin/qmake
 • qmake -v → Qt version 6.9.3
 • CMAKE_PREFIX_PATH → /opt/Qt/6.9.3/gcc_64/lib/cmake

⸻

5) Smoke test build

Run a minimal Qt build to verify toolchain + environment.

mkdir -p ~/qt-smoke && cd ~/qt-smoke

cat > CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.22)
project(QtSmoke LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
add_executable(qt_smoke main.cpp)
target_link_libraries(qt_smoke PRIVATE Qt6::Widgets)
EOF

cat > main.cpp <<'EOF'
# include <QApplication>
# include <QLabel>
int main(int argc, char** argv){
  QApplication a(argc, argv);
  QLabel l("Qt smoke ✅");
  return 0;
}
EOF

cmake -G Ninja .
ninja -k0

If the build succeeds and produces a qt_smoke executable, Qt is installed and wired correctly.

⸻

Troubleshooting
 • qmake: command not found: Ensure /etc/environment has been updated and reload your shell with bash -l.
 • CMake cannot find Qt: Ensure CMAKE_PREFIX_PATH points to /opt/Qt/6.9.3/gcc_64/lib/cmake.
 • Missing OpenGL headers: Run sudo dnf install -y mesa-libGL-devel libglvnd-devel.

⸻

Next Steps

After the environment is verified, clone and build project repositories:

cd ~
git clone <https://github.com/DesignOpticsFast/phoenix.git>
cd phoenix
mkdir build && cd build
cmake .. -G Ninja
ninja -k0

This completes the dev environment setup for Phoenix, Bedrock, and Rosetta.
