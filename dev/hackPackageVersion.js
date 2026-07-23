import { writeFileSync } from "fs";
import pkg from "../package.json" with { type: "json" };

writeFileSync("./package.json", JSON.stringify(pkg), "utf8");
