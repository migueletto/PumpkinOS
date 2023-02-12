-- initialization script

function command_eval(cmd)
  local s,r,flag = nil,"Compilation error.",false
  local f = load("return " .. cmd)
  if not f then
    f = load(cmd)
    flag = true
  end
  if f then
    s,r = pcall(f)
    if s then
      if not r and flag then
        r = ""
      end
    else
      r = "Execution error."
    end
  end
  return r
end
