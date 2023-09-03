import os


def main():
    compile_shaders()


def compile_shaders():
    project_root = os.path.abspath(os.path.join(os.getcwd(), os.pardir))
    shader_dir = os.path.join(project_root, "shaders")
    spirv_dir = os.path.join(project_root, "spirv")

    assert(os.path.exists(shader_dir))

    if not os.path.exists(spirv_dir):
        os.mkdir(spirv_dir)

    for file in os.listdir(shader_dir):
        file_path = os.path.join(shader_dir, file)
        os.system(f"glslangValidator -V {file_path}")

    for file in os.listdir(os.getcwd()):
        if file.endswith(".spv"):
            file_path = os.path.join(os.getcwd(), file)
            new_path = os.path.join(spirv_dir, file)
            os.replace(file_path, new_path)


if __name__ == "__main__":
    main()