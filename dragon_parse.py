import pathlib
import pandas as pd

folder = pathlib.Path(__file__).parent.absolute()
input_folder = folder / "Maggio_2023"
output_folder = folder / "may23"

spire = {
    "0.127 4.45 6 1": 341,
    "0.127 4.46 6 1": 297,
    "4.46 0.127 6 1": 76,
    "4.46 4.45 4 1": 67,
    "4.45 4.46 8 1": 87,
    "0.127 4.47 6 1": 254,
    "0.127 4.46 2 1 ": 276,
    "4.45 4.44 4 1": 111,
    "4.46 4.47 8 1": 65,
    "0.127 4.44 2 1": 363,
    "4.45 4.46 8 1": 68,
    "4.41 4.42 4 1": 155,
    "4.44 4.41 4 1": 133,
    "4.41 4.33 6 1": 166,
    "0.127 4.41 6 1": 427,
    "2.6 2.10 6 1": 21,
    "4.47 4.46 4 1 ": 45,
    "4.44 4.45 8 1": 131,
    "2.5 2.6 2 1": 190,
    "4.47 2.6 8 1": 43,
    "2.6 4.47 4 1 ": 23,
    "2.10 2.6 6 1": 1,
    "4.42 4.41 8 1": 175,
    "0.127 4.44 6 1": 384,
    "2.6 2.5 6 1": 30,
    "0.127 4.47 2 1": 233,
    "4.41 4.44 8 1": 153,
}

for file in input_folder.iterdir():
    if file.is_file() and file.suffix == ".csv":
        df = pd.read_csv(file, sep=",")
        df = df[df["is_working"].astype(bool)]
        for index, row in df.iterrows():
            if not row["section"].strip() in spire:
                df.drop(index, inplace=True)
        df = df.reset_index(drop=True)
        df = df.drop("is_working", axis=1)
        df["data"] = (
            df["data"]
            .apply(lambda x: x.split("[")[1].split("]")[0])
            .str.replace(",", "")
            .replace("  ", " ")
        )
        df.to_csv(output_folder / file.name, index=False, sep=";")
