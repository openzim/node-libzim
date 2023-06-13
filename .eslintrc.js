module.exports = {
  env: {
    es6: true,
    node: true,
  },
  rules:{
    "prettier/prettier" : "error",
    "eqeqeq": "error"
  },
  extends: ["prettier"],
  parser: "@typescript-eslint/parser",
  parserOptions: {
    project: "tsconfig.json",
    sourceType: "module",
  },
  plugins: ["prettier"],
  root: true,
  overrides: [
    {
      files: ["*.ts"],
      rules: {
        "@typescript-eslint/no-var-requires" : "off"
      },
      parserOptions: {
        sourceType: "module"
      },
      extends : ["plugin:@typescript-eslint/recommended"],
       parser: "@typescript-eslint/parser",
      plugins: ["@typescript-eslint"]
    }
  ],
};
