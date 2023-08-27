from sys import platform
import os


try:
    os.makedirs("spirv")
except FileExistsError:
    pass


for file in os.listdir("shaders"):
    if platform in ["linux", "cygwin","darwin"]:
        os.system(f"glslangValidator -V shaders/{file}")
        os.system("mv -f *.spv ./spirv")

    elif platform == "win32":
        raise Exception("TODO")

    else:
        raise Exception("Unsupported platform")
