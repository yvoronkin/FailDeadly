Import("env")

from pathlib import Path
import shlex
import subprocess


project_dir = Path(env.subst("$PROJECT_DIR")).resolve()
source_dir = (project_dir/"lib"/"MbedTLS"/"mbedtls"/"tf-psa-crypto")
build_dir = (Path(env.subst("$BUILD_DIR"))/"tf-psa-crypto")
config_file = (project_dir/"config"/"faildeadly_tf_psa_crypto_config.h")
python_executable = (project_dir/"venv"/"bin"/"python")

toolchain_file = build_dir / "platformio-toolchain.cmake"

def shell_join(values):
    return " ".join(shlex.quote(str(value)) for value in values)

def generate_toolchain():
    build_dir.mkdir(parents=True, exist_ok=True)

    toolchain_dir = Path(env.subst("$PROJECT_PACKAGES_DIR") + "/toolchain-gccarmnoneeabi/bin/").resolve()
    toolchain_name = "arm-none-eabi-"

    cc = Path(toolchain_dir / (toolchain_name + "gcc")).resolve()
    ar = Path(toolchain_dir / (toolchain_name + "ar")).resolve()
    ranlib = Path(toolchain_dir / (toolchain_name + "ranlib")).resolve()

    # STM32F401 / Cortex-M4F.
    architecture_flags = [
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfpu=fpv4-sp-d16",
        "-mfloat-abi=softfp",
        "-ffunction-sections",
        "-fdata-sections",
    ]

    c_flags = shell_join(architecture_flags)

    contents = f"""
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER "{cc}")
set(CMAKE_AR "{ar}")
set(CMAKE_RANLIB "{ranlib}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_C_FLAGS_INIT "{c_flags}")
"""

    toolchain_file.write_text(contents.strip() + "\n")

def configure():
    command = [
        "cmake",
        "-S", str(source_dir),
        "-B", str(build_dir),

        f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
        f"-DPython3_EXECUTABLE={python_executable}",
        f"-DTF_PSA_CRYPTO_PYTHON_EXECUTABLE={python_executable}",

        "-DCMAKE_BUILD_TYPE=MinSizeRel",

        "-DENABLE_PROGRAMS=OFF",
        "-DENABLE_TESTING=OFF",

        "-DUSE_STATIC_TF_PSA_CRYPTO_LIBRARY=ON",
        "-DUSE_SHARED_TF_PSA_CRYPTO_LIBRARY=OFF",

        "-DDISABLE_PACKAGE_CONFIG_AND_INSTALL=ON",

        f"-DTF_PSA_CRYPTO_CONFIG_FILE={config_file}",
    ]

    subprocess.run(command, check=True)


def build():
    subprocess.run(
        [
            "cmake",
            "--build", str(build_dir),
            "--target", "tfpsacrypto",
            "--parallel",
        ],
        check=True,
    )

def locate_archive():
    candidates = list(build_dir.rglob("libtfpsacrypto.a"))

    if len(candidates) != 1:
        raise RuntimeError(
            "Expected exactly one libtfpsacrypto.a, found: "
            + ", ".join(str(path) for path in candidates)
        )

    return candidates[0]

print(env.subst("$TARGET"))

generate_toolchain()
configure()
build()

archive = locate_archive()

env.Append(
    LIBPATH=[str(archive.parent)],
    LIBS=["tfpsacrypto"],
)
