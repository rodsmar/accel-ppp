ADD_LIBRARY(ipv6 SHARED
	ipv6.c
	dhcpv6.c
	dhcpv6_packet.c
	dhcpv6_match.c
	nd.c
)

INSTALL(TARGETS ipv6_dhcp ipv6_nd
	LIBRARY DESTINATION lib${LIB_SUFFIX}/accel-ppp
)

