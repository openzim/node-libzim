import { writeFileSync } from "fs";
import pkg from "../package.json";

writeFileSync("./package.json", JSON.stringify(pkg), "utf8");
