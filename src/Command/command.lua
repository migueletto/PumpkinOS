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

function ls(dir, f)
  if not dir then dir = "." end
  local t = nil
  local d = opendir(dir)
  if d then
    while true do
      local e = readdir(d)
      if not e then break end
      if f and type(f) == "function" then
        if not t then t = {} end
        f(t, e)
      else
        local dt, dr
        if e.directory then dt = "d"  else dt = " "  end
        if e.readOnly  then dr = "r " else dr = "w " end
        print(dt .. dr .. e.name)
      end
    end
    closedir(d)
  end
  return t
end

function lsdb(dbtype, creator, pattern)
  local d = opendb(dbtype, creator)
  local t = nil
  if d then
    while true do
      local e = readdb(d)
      if not e then break end
      local ok
      if pattern then
        if type(pattern) == "function" then
          if not t then t = {} end
          pattern(t, e)
          ok = false
        else
          ok = string.match(e.name, pattern)
        end
      else
        ok = true
      end
      if ok then
        print(e.type .. " " .. e.creator .. " " .. e.name)
      end
    end
    closedb(d)
  end
  return t
end
