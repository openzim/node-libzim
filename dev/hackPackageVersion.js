import { writeFileSync } from "fs";
import pkg, { version } from "../package.json";

writeFileSync("./package.json", JSON.stringify(pkg), "utf8");
