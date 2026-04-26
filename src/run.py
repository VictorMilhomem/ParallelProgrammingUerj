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
    
    def run(self, k: int, n: int, input_script: str = "geninput.py") -> None:
        base_path = Path(__file__).parent.resolve()

        c_filepath = base_path / self.c_code
        output_exe = base_path / Path(self.c_code).stem

        input_file = base_path / "input.txt"
        output_file = base_path / "output.txt"
        input_script_path = base_path / input_script

        # Gen input.txt -> kmeans specific
        with open(input_file, "w") as f:
            subprocess.run(
                ["python3", str(input_script_path), str(k), str(n)],
                stdout=f,
                check=True
            )

        subprocess.run(
            ["clang", "-o", str(output_exe), "-O3", str(c_filepath), "-lm", "-fopenmp"],
            check=True
        )

        # Run the executable with input.txt and save output to output.txt
        with open(input_file, "r") as fin, open(output_file, "w") as fout:
            subprocess.run(
                [str(output_exe), str(self.runs), str(n)],
                stdin=fin,
                stdout=fout,
                check=True,
                cwd=base_path 
            )

        os.remove(output_exe)

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
        plt.savefig(save_to)
        plt.show()


if __name__ == "__main__":
    sim = Simulation(c_code="main.c", runs=10)

    sim.run(k=50, n=1000000)

    base_path = Path(__file__).parent.resolve()
    csv_path = base_path / "speedup.csv"

    data = sim.speedup(csv_path)
    sim.plot(data, base_path / "kmeans_speedup.png")