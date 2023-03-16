// initialization script

function command_eval(cmd) {
  try {
    return eval(cmd);
  } catch (e) {
    return e.toString();
  }
}

function test() {
  deploy("Test", "Test", "/PALM/Programs/Command/test.js");
}
