import js from "@eslint/js";
import tsPlugin from "@typescript-eslint/eslint-plugin";
import tsParser from "@typescript-eslint/parser";
import prettierPlugin from "eslint-plugin-prettier";
import prettierConfig from "eslint-config-prettier";
import globals from "globals";

export default [
  {
    ignores: [
      "node_modules/**",
      "dist/**",
      "build/**",
      "download/**",
      "eslint.config.js",
    ],
  },
  js.configs.recommended,
  {
    files: ["**/*.ts", "**/*.js"],
    plugins: {
      "@typescript-eslint": tsPlugin,
      prettier: prettierPlugin,
    },
    languageOptions: {
      parser: tsParser,
      parserOptions: {
        project: "tsconfig.json",
        sourceType: "module",
      },
      globals: {
        ...globals.node,
        ...globals.es2015,
      },
    },
    rules: {
      ...tsPlugin.configs["eslint-recommended"].overrides[0].rules,
      ...tsPlugin.configs.recommended.rules,
      ...prettierConfig.rules,
      "prettier/prettier": "error",
      eqeqeq: "error",
      "@typescript-eslint/no-var-requires": "off",
      "no-import-assign": "off",
      "no-useless-escape": "off",
    },
  },
];
