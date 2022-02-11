require "vstudio"

premake.api.register {
    name = "customtargets",
    scope = "project",
    kind  = "list:string",
}


local function addCustomTargets(prj)
    if prj.customtargets == nil or #prj.customtargets == 0 then
        return
    end

    for _,v in ipairs(prj.customtargets) do
        premake.w(deindent(v))
    end
end

premake.override(premake.vstudio.vc2010.elements, "project", function(base, prj)
    local calls = base(prj)
    table.insert(calls, addCustomTargets)
    return calls
end)