-- hoge packege --
module("hoge",package.seeall)

value=2

function box_muller()
	local alpha=math.random()	-- package.seeall �̂������Ŏg����
	local beta =math.random()
	
	return math.sqrt(-2*math.log(alpha))*math.sin(2*math.pi*beta)
end


