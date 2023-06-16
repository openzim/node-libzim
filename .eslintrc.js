module.exports = {
  env: {
    es6: true,
    node: true,
  },
  rules: {
    "prettier/prettier": "error",
    eqeqeq: "error",
  },
  extends: [
    "prettier",
    "eslint:recommended",
    "plugin:@typescript-eslint/eslint-recommended",
    "plugin:@typescript-eslint/recommended",
  ],
  parser: "@typescript-eslint/parser",
  parserOptions: {
    project: "tsconfig.json",
    sourceType: "module",
  },
  plugins: ["prettier"],
  root: true,
  overrides: [
    {
      files: ["*.ts", "*.js"],
      rules: {
        "@typescript-eslint/no-var-requires": "off",
        "no-import-assign": "off",
        "no-useless-escape": "off",
      },
      parserOptions: {
        sourceType: "module",
      },
      extends: ["plugin:@typescript-eslint/recommended"],
      parser: "@typescript-eslint/parser",
      plugins: ["@typescript-eslint"],
    },
  ],
};
