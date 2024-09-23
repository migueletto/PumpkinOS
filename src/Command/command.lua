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

function chain(list)
  if list and type(list) == "table" then
    local n = #list
    local input = nil
    local output = nil
    setin(nil)
    setout(nil)
    for i = 1,n do
      local current = list[i]
      local next = list[i+1]
      if current then
        if type(current) == "function" then
          if next and type(next) == "function" then
            output = string.format("tmp%d_%d", tid(), i)
          else
            output = nil
          end
          setout(output)
          current()
          if input then
            setin(nil)
            rm(input)
          end
          input = output
          setin(input)
          output = nil
          setout(output)
        end
      end
    end
  end
end

function ls(dir, f)
  if not dir then dir = "." end
  local t = nil
  local sorted = nil
  local d = opendir(dir)
  if d then
    while true do
      local e = readdir(d)
      if not e then break end
      if f and type(f) == "function" then
        if not t then t = {} end
        f(t, e)
      else
        if not sorted then sorted = {} end
        table.insert(sorted, e)
      end
    end
    closedir(d)
    if sorted then
      table.sort(sorted,
        function(e1, e2)
          return string.lower(e1.name) < string.lower(e2.name)
        end)
      for k,e in pairs(sorted) do
        local dt, dr
        if e.directory then dt = "d"  else dt = " "  end
        if e.readOnly  then dr = "r " else dr = "w " end
        print(dt .. dr .. e.name)
      end
    end
  end
  return t
end

function lsdb(dbtype, creator, pattern)
  local d = opendb(dbtype, creator)
  local t = nil
  local sorted = nil
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
        if not sorted then sorted = {} end
        table.insert(sorted, e)
      end
    end
    closedb(d)
    if sorted then
      table.sort(sorted,
        function(e1, e2)
          return string.lower(e1.name) < string.lower(e2.name)
        end)
      for k,e in pairs(sorted) do
        print(e.type .. " " .. e.creator .. " " .. e.name)
      end
    end
  end
  return t
end
