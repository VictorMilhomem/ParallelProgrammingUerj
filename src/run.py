import subprocess
import matplotlib.pyplot as plt
from dataclasses import dataclass
from pathlib import Path
import os
import csv
from collections import defaultdict

@dataclass
class Simulation:
    c_code: str
    runs: int

    def run(self) -> None:
        filepath = os.path.join(Path(__file__).parent.resolve(), self.c_code)
        output_exe = os.path.join(Path(__file__).parent.resolve(), self.c_code.split(".")[0])

        compile_cmd = ["gcc", "-o", output_exe, "-O3", filepath, "-fopenmp"]
        subprocess.run(compile_cmd, check=True)

        exec_cmd = [output_exe, f"{self.runs}"]
        subprocess.run(args=exec_cmd)

        clean_cmd = ["rm", output_exe]
        subprocess.run(args=clean_cmd)

    def speedup(self, filepath: str) -> dict:
        data = defaultdict(list)
        with open(filepath, newline='') as f:
            reader = csv.DictReader(f)
            for row in reader:
                n = int(row["n_row"])
                threads = int(row["threads"])
                speedup = float(row["speedup"])

                data[n].append((threads, speedup))

        return data

    def plot(self, data, save_to):
        for n, values in sorted(data.items()):
            values.sort(key=lambda x: x[0])
            threads = [v[0] for v in values]
            speedups = [v[1] for v in values]

            plt.plot(threads, speedups, marker='o', label=f"N={n}")

        plt.title("Speedup vs Threads")
        plt.xlabel("Threads")
        plt.ylabel("Speedup")
        plt.grid()
        plt.legend()
        plt.show()
        plt.savefig(save_to)

if __name__ == "__main__":
    sim = Simulation(c_code="main.c", runs=10)
    sim.run()
    # running outside src
    csv_path = os.path.join(Path(__file__).parent.parent.resolve(), "speedup.csv")

    data = sim.speedup(csv_path)
    sim.plot(data, "matmul_speedup.png")
