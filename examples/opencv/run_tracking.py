
from ecospy import *

from pathlib import Path
from time import sleep

def read_png_file_to_bytes(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()

def main():
    print(f"Ecoslib version: {EcosLib.version()}")

    EcosLib.set_log_level("debug")

    fmu_path =  str((Path(__file__).parent.parent.parent / 'cmake-build-debug' / 'models' /'fmi3' / 'onnx_tracker' / 'onnx_tracker.fmu').resolve())
    image = read_png_file_to_bytes("Lenna_(test_image).png")

    print(f"image size: {len(image)} bytes")

    with EcosSimulationStructure() as ss:
        ss.add_model("model", fmu_path)

        with(EcosSimulation(structure=ss, step_size=1/100)) as sim:

            sim.init()
            sim.set_binary("model::blob", image)
            sim.step()
            sleep(1)
            sim.terminate()



if __name__ == "__main__":
    main()
