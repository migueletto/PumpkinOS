-- initialization script

function command_eval(cmd)
  local s,flag = nil,false
  local f,r = load("return " .. cmd)
  if not f then
    f,r = load(cmd)
    flag = true
  end
  if f then
    s,r = pcall(f)
    if s then
      if not r and flag then
        r = ""
      end
    end
  end
  return r
end
