-- hoge packege --
module("hoge",package.seeall)

value=2

function box_muller()
	local alpha=math.random()	-- package.seeall のおかげで使える
	local beta =math.random()
	
	return math.sqrt(-2*math.log(alpha))*math.sin(2*math.pi*beta)
end


