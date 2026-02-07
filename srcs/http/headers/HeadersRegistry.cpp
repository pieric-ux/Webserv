// TODO: don't forget header

/**
* @file HeadersRegistry.cpp
* @brief [TODO:description]
*/

#include <webserv/HeadersRegistry.hpp>
#include <algorithm>
#include <cctype>

namespace http
{
namespace headers
{

/**
 * @brief [TODO:description]
 * @link https://www.iana.org/assignments/http-fields/http-fields.xhtml
 */
HeadersRegistry::HeadersRegistry()
{
	// A
	_headers["a-im"] = Headers("A-IM", "Used with Response code to indicate partial instance manipulations.");
	_headers["accept"] = Headers("Accept", "Informs the server about the types of data that can be sent back.");
	_headers["accept-additions"] = Headers("Accept-Additions", "Indicates acceptance of additions to the resource.");
	_headers["accept-ch"] = Headers("Accept-CH", "Servers advertise support for Client Hints using this header.");
	_headers["accept-charset"] = Headers("Accept-Charset", "Informs the server about which character sets can be used in the response.");
	_headers["accept-datetime"] = Headers("Accept-Datetime", "Informs the server about the preferred version of a resource based on datetime.");
	_headers["accept-encoding"] = Headers("Accept-Encoding", "The encoding algorithm that can be used on the resource sent back.");
	_headers["accept-features"] = Headers("Accept-Features", "Indicates which features the client supports.");
	_headers["accept-language"] = Headers("Accept-Language", "Informs the server about the human language the server is expected to send back.");
	_headers["accept-patch"] = Headers("Accept-Patch", "Advertises which media type the server is able to understand in a PATCH request.");
	_headers["accept-post"] = Headers("Accept-Post", "Advertises which media type the server is able to understand in a POST request.");
	_headers["accept-query"] = Headers("Accept-Query", "Advertises which media type the server is able to understand in a QUERY request.");
	_headers["accept-ranges"] = Headers("Accept-Ranges", "Indicates if the server supports range requests.");
	_headers["accept-signature"] = Headers("Accept-Signature", "Indicates intention to take advantage of any available signatures.");
	_headers["access-control"] = Headers("Access-Control", "Advertises which control features the server supports.");
	_headers["access-control-allow-credentials"] = Headers("Access-Control-Allow-Credentials", "Indicates whether the response can be exposed when credentials flag is true.");
	_headers["access-control-allow-headers"] = Headers("Access-Control-Allow-Headers", "Indicates which headers can be used in the actual request.");
	_headers["access-control-allow-methods"] = Headers("Access-Control-Allow-Methods", "Specifies the methods allowed when accessing the resource.");
	_headers["access-control-allow-origin"] = Headers("Access-Control-Allow-Origin", "Indicates whether the response can be shared.");
	_headers["access-control-expose-headers"] = Headers("Access-Control-Expose-Headers", "Indicates which headers can be exposed as part of the response.");
	_headers["access-control-max-age"] = Headers("Access-Control-Max-Age", "Indicates how long preflight request results can be cached.");
	_headers["access-control-request-headers"] = Headers("Access-Control-Request-Headers", "Used in preflight requests to indicate which headers will be used.");
	_headers["access-control-request-method"] = Headers("Access-Control-Request-Method", "Used in preflight requests to indicate which method will be used.");
	_headers["activate-storage-access"] = Headers("Activate-Storage-Access", "Indicates activation of storage access permission.");
	_headers["age"] = Headers("Age", "Time in seconds that the object has been in a proxy cache.");
	_headers["allow"] = Headers("Allow", "Lists the set of HTTP request methods supported by a resource.");
	_headers["alpn"] = Headers("ALPN", "Used to indicate which protocol should be used over TLS.");
	_headers["alt-svc"] = Headers("Alt-Svc", "Used to list alternate ways to reach this service.");
	_headers["alt-used"] = Headers("Alt-Used", "Used to identify the alternative service in use.");
	_headers["alternates"] = Headers("Alternates", "Used to indicate alternative versions of a resource.");
	_headers["amp-cache-transform"] = Headers("AMP-Cache-Transform", "Indicates transformations applied by AMP cache.");
	_headers["apply-to-redirect-ref"] = Headers("Apply-To-Redirect-Ref", "Indicates whether the referrer should be included in redirected requests.");
	_headers["authentication-control"] = Headers("Authentication-Control", "Controls authentication behavior.");
	_headers["authentication-info"] = Headers("Authentication-Info", "Provides authentication information in response to authentication.");
	_headers["authorization"] = Headers("Authorization", "Contains the credentials to authenticate a user-agent with a server.");
	_headers["attribution-reporting-eligible"] = Headers("Attribution-Reporting-Eligible", "Marks the response eligible for attribution reporting.");
	_headers["attribution-reporting-register-source"] = Headers("Attribution-Reporting-Register-Source", "Registers an attribution reporting source.");
	_headers["attribution-reporting-register-trigger"] = Headers("Attribution-Reporting-Register-Trigger", "Registers an attribution reporting trigger.");
	_headers["available-dictionary"] = Headers("Available-Dictionary", "Indicates which dictionaries are available for compression.");
	
	// B - C
	_headers["c-ext"] = Headers("C-Ext", "Used to indicate support for experimental features.");
	_headers["c-man"] = Headers("C-Man", "Used to indicate the presence of an experimental feature.");
	_headers["c-opt"] = Headers("C-Opt", "Used to indicate support for optional features.");
	_headers["c-pep"] = Headers("C-PEP", "Used to indicate support for a specific protocol extension.");
	_headers["c-pep-info"] = Headers("C-PEP-Info", "Used to provide information about a specific protocol extension.");
	_headers["cache-control"] = Headers("Cache-Control", "Directives for caching mechanisms in both requests and responses.");
	_headers["cache-group-invalidation"] = Headers("Cache-Group-Invalidation", "Indicates that a cache group is invalidated.");
	_headers["cache-groups"] = Headers("Cache-Groups", "Indicates the cache groups that a response belongs to.");
	_headers["cache-status"] = Headers("Cache-Status", "Indicates cache status of the response.");
	_headers["cal-managed-id"] = Headers("Cal-Managed-ID", "Used in calendaring to indicate managed identifiers.");
	_headers["caldav-timezones"] = Headers("CalDAV-Timezones", "Used in calendaring to indicate timezone information.");
	_headers["capsule-protocol"] = Headers("Capsule-Protocol", "Used to indicate support for capsule protocols.");
	_headers["cdn-cache-control"] = Headers("CDN-Cache-Control", "Cache directives targeted at content delivery networks.");
	_headers["cdn-loop"] = Headers("CDN-Loop", "Used for loop detection in Content Delivery Networks.");
	_headers["cert-not-after"] = Headers("Cert-Not-After", "Indicates the certificate validity end time.");
	_headers["cert-not-before"] = Headers("Cert-Not-Before", "Indicates the certificate validity start time.");
	_headers["clear-site-data"] = Headers("Clear-Site-Data", "Instructs the user agent to clear browsing data.");
	_headers["client-cert"] = Headers("Client-Cert", "Client certificate as a header field.");
	_headers["client-cert-chain"] = Headers("Client-Cert-Chain", "Client certificate chain as a header field.");
	_headers["close"] = Headers("Close", "Indicates that the connection should be closed after completion of the response.");
	_headers["cmcd-object"] = Headers("CMCD-Object", "Used in Common Media Client Data to provide object-level information.");
	_headers["cmcd-request"] = Headers("CMCD-Request", "Used in Common Media Client Data to provide request-level information.");
	_headers["cmcd-session"] = Headers("CMCD-Session", "Used in Common Media Client Data to provide session-level information.");
	_headers["cmcd-status"] = Headers("CMCD-Status", "Used in Common Media Client Data to provide status information.");
	_headers["cmsd-dynamic"] = Headers("CMSD-Dynamic", "Used in Common Media Server Data to indicate dynamic information.");
	_headers["cmsd-static"] = Headers("CMSD-Static", "Used in Common Media Server Data to indicate static information.");
	_headers["concealed-auth-export"] = Headers("Concealed-Auth-Export", "Used to indicate that authentication information is concealed.");
	_headers["configuration-context"] = Headers("Configuration-Context", "Used to indicate the context of a configuration.");
	_headers["connection"] = Headers("Connection", "Controls whether the connection stays open after the transaction.");
	_headers["content-base"] = Headers("Content-Base", "Deprecated header that specifies the base URI for resolving relative URLs in the response.");
	_headers["content-digest"] = Headers("Content-Digest", "Provides a digest of the message content.");
	_headers["content-disposition"] = Headers("Content-Disposition", "Indicates if resource should be displayed inline or as download.");
	_headers["content-encoding"] = Headers("Content-Encoding", "Used to specify the compression algorithm.");
	_headers["content-id"] = Headers("Content-ID", "Used to identify a specific part of a multipart message.");
	_headers["content-language"] = Headers("Content-Language", "Describes the human language(s) intended for the audience.");
	_headers["content-length"] = Headers("Content-Length", "The size of the resource in decimal number of bytes.");
	_headers["content-location"] = Headers("Content-Location", "Indicates an alternate location for the returned data.");
	_headers["content-md5"] = Headers("Content-MD5", "A Base64-encoded MD5 digest of the message content.");
	_headers["content-range"] = Headers("Content-Range", "Indicates where in a full body a partial message belongs.");
	_headers["content-script-type"] = Headers("Content-Script-Type", "Used to specify the scripting language of the resource.");
	_headers["content-security-policy"] = Headers("Content-Security-Policy", "Controls resources the user agent is allowed to load.");
	_headers["content-security-policy-report-only"] = Headers("Content-Security-Policy-Report-Only", "Allows monitoring CSP violations without enforcing.");
	_headers["content-style-type"] = Headers("Content-Style-Type", "Used to specify the styling language of the resource.");
	_headers["content-type"] = Headers("Content-Type", "Indicates the media type of the resource.");
	_headers["content-version"] = Headers("Content-Version", "Used to indicate the version of the content.");
	_headers["cookie"] = Headers("Cookie", "Contains stored HTTP cookies previously sent by the server.");
	_headers["cookie2"] = Headers("Cookie2", "Obsolete header that was used to indicate support for the newer cookie specification.");
	_headers["cross-origin-embedder-policy"] = Headers("Cross-Origin-Embedder-Policy", "Declares an embedder policy for a document.");
	_headers["cross-origin-embedder-policy-report-only"] = Headers("Cross-Origin-Embedder-Policy-Report-Only", "Declares an embedder policy for a document without enforcing it.");
	_headers["cross-origin-opener-policy"] = Headers("Cross-Origin-Opener-Policy", "Prevents other domains from opening/controlling a window.");
	_headers["cross-origin-opener-policy-report-only"] = Headers("Cross-Origin-Opener-Policy-Report-Only", "Prevents other domains from opening/controlling a window without enforcing it.");
	_headers["cross-origin-resource-policy"] = Headers("Cross-Origin-Resource-Policy", "Prevents other domains from reading the response.");
	_headers["cta-common-access-token"] = Headers("CTA-Common-Access-Token", "Used in Common Token Access to indicate a common access token.");

	// D - E
	_headers["dasl"] = Headers("DASL", "Used in WebDAV to indicate a DASL query.");
	_headers["date"] = Headers("Date", "Contains the date and time at which the message was originated.");
	_headers["dav"] = Headers("DAV", "Used in WebDAV to indicate compliance with DAV protocol.");
	_headers["default-style"] = Headers("Default-Style", "Used to indicate the default style for a resource.");
	_headers["delta-base"] = Headers("Delta-Base", "Used to indicate the base URI for delta encoding.");
	_headers["deprecation"] = Headers("Deprecation", "Indicates that the resource is deprecated.");
	_headers["depth"] = Headers("Depth", "Used in WebDAV requests to specify the depth of operation.");
	_headers["derived-from"] = Headers("Derived-From", "Used to indicate the source of a derived resource.");
	_headers["destination"] = Headers("Destination", "Used in WebDAV to specify the destination resource.");
	_headers["detached-jws"] = Headers("Detached-JWS", "Used to indicate a detached JSON Web Signature.");
	_headers["differential-id"] = Headers("Differential-ID", "Used to indicate the identifier for a differential version of a resource.");
	_headers["dictionary-id"] = Headers("Dictionary-ID", "Used to indicate the identifier for a compression dictionary.");
	_headers["digest"] = Headers("Digest", "Used to indicate a digest of the message content.");
	_headers["dnt"] = Headers("DNT", "Indicates the user's tracking preference (Do Not Track).");
	_headers["dpop"] = Headers("DPoP", "Used to indicate a DPoP proof JWT for binding tokens to a TLS connection.");
	_headers["dpop-nonce"] = Headers("DPoP-Nonce", "Used to indicate a nonce for DPoP proof JWTs.");
	_headers["early-data"] = Headers("Early-Data", "Indicates the request has been conveyed in TLS early data.");
	_headers["ediint-features"] = Headers("EDIINT-Features", "Used to indicate support for EDIINT features.");
	_headers["etag"] = Headers("ETag", "A unique string identifying the version of the resource.");
	_headers["expect"] = Headers("Expect", "Indicates expectations that need to be fulfilled by the server.");
	_headers["expect-ct"] = Headers("Expect-CT", "Used to indicate that the site expects Certificate Transparency compliance.");
	_headers["expires"] = Headers("Expires", "The date/time after which the response is considered stale.");
	_headers["ext"] = Headers("Ext", "Used to indicate support for extensions to the protocol.");

	// F - G
	_headers["forwarded"] = Headers("Forwarded", "Contains information from the client-facing side of proxy servers.");
	_headers["from"] = Headers("From", "Contains an Internet email address for a human user.");
	_headers["getprofile"] = Headers("GetProfile", "Used to indicate a profile to be used in a GET request.");

	// H - I
	_headers["hobareg"] = Headers("Hobareg", "Used in WebDAV to indicate a resource that is a collection.");
	_headers["host"] = Headers("Host", "Specifies the domain name of the server (for virtual hosting).");
	_headers["http2-settings"] = Headers("HTTP2-Settings", "Used to indicate HTTP/2 connection parameters.");
	_headers["if"] = Headers("If", "Used in WebDAV for conditional operations.");
	_headers["if-match"] = Headers("If-Match", "Conditional request that applies only if resource matches ETags.");
	_headers["if-modified-since"] = Headers("If-Modified-Since", "Conditional request transmitting if resource modified after date.");
	_headers["if-none-match"] = Headers("If-None-Match", "Conditional request if resource doesn't match ETags.");
	_headers["if-range"] = Headers("If-Range", "Conditional range request fulfilled if etag or date matches.");
	_headers["if-schedule-tag-match"] = Headers("If-Schedule-Tag-Match", "Used in calendaring to indicate conditional request based on schedule tag.");
	_headers["if-unmodified-since"] = Headers("If-Unmodified-Since", "Conditional request if resource not modified after date.");
	_headers["idempotency-key"] = Headers("Idempotency-Key", "Provides a unique key for idempotent POST/PATCH requests.");
	_headers["im"] = Headers("IM", "Instance manipulation used with response codes.");
	_headers["include-referred-token-binding-id"] = Headers("Include-Referred-Token-Binding-ID", "Used to indicate that the referred token binding ID should be included.");
	_headers["integrity-policy"] = Headers("Integrity-Policy", "Ensures resources have Subresource Integrity guarantees.");
	_headers["integrity-policy-report-only"] = Headers("Integrity-Policy-Report-Only", "Reports resources that would violate integrity policy.");
	_headers["isolation"] = Headers("Isolation", "Used to indicate the isolation level of a resource.");

	// K - L
	_headers["keep-alive"] = Headers("Keep-Alive", "Controls how long a persistent connection should stay open.");
	_headers["label"] = Headers("Label", "Used to indicate a label for the resource.");
	_headers["last-event-id"] = Headers("Last-Event-ID", "Used in Server-Sent Events to indicate the ID of the last event received.");
	_headers["last-modified"] = Headers("Last-Modified", "The date and time the resource was last modified.");
	_headers["link"] = Headers("Link", "Provides a means for serializing one or more links in headers.");
	_headers["link-template"] = Headers("Link-Template", "Provides templated links.");
	_headers["location"] = Headers("Location", "Indicates the URL to redirect a page to.");
	_headers["lock-token"] = Headers("Lock-Token", "Used in WebDAV to indicate a lock token.");

	// M - N
	_headers["man"] = Headers("Man", "Used to indicate support for a specific protocol extension.");
	_headers["max-forwards"] = Headers("Max-Forwards", "Indicates maximum number of hops for TRACE method.");
	_headers["memento-datetime"] = Headers("Memento-Datetime", "Used for time-based access to resource states.");
	_headers["meter"] = Headers("Meter", "Used to indicate a metering requirement for the resource.");
	_headers["method-check"] = Headers("Method-Check", "Used to indicate that the server should check the method of the request.");
	_headers["method-check-expires"] = Headers("Method-Check-Expires", "Used to indicate the time after which the method check should be considered expired.");
	_headers["mime-version"] = Headers("MIME-Version", "Indicates the version of MIME used in the message.");
	_headers["negotiate"] = Headers("Negotiate", "Used to indicate support for content negotiation.");
	_headers["nel"] = Headers("NEL", "Defines a network error reporting policy.");
	_headers["no-vary-search"] = Headers("No-Vary-Search", "Defines how query parameters affect cache matching.");

	// O
	_headers["odata-entityid"] = Headers("OData-EntityId", "Used in OData to indicate the entity ID of a resource.");
	_headers["odata-isolation"] = Headers("OData-Isolation", "Used in OData to indicate the isolation level of a resource.");
	_headers["odata-maxversion"] = Headers("OData-MaxVersion", "Used in OData to indicate the maximum OData version supported by the client.");
	_headers["odata-version"] = Headers("OData-Version", "Used in OData to indicate the OData version of the message.");
	_headers["opt"] = Headers("Opt", "Used to indicate support for optional features.");
	_headers["optional-www-authenticate"] = Headers("Optional-WWW-Authenticate", "Used to indicate optional authentication methods.");
	_headers["ordering-type"] = Headers("Ordering-Type", "Used to indicate the ordering type of a resource.");
	_headers["observe-browsing-topics"] = Headers("Observe-Browsing-Topics", "Marks topics inferred from a request as observed.");
	_headers["origin"] = Headers("Origin", "Indicates where a fetch originates from.");
	_headers["origin-agent-cluster"] = Headers("Origin-Agent-Cluster", "Indicates origin-keyed agent cluster placement.");
	_headers["oscore"] = Headers("OSCORE", "Used to indicate support for OSCORE protocol.");
	_headers["oslc-core-version"] = Headers("OSLC-Core-Version", "Used to indicate the version of OSLC core supported.");
	_headers["overwrite"] = Headers("Overwrite", "Used in WebDAV to control overwrite on copy/move.");

	// P - R
	_headers["p3p"] = Headers("P3P", "Used to indicate the P3P policy of the server.");
	_headers["pep"] = Headers("PEP", "Used to indicate support for a specific protocol extension.");
	_headers["pep-info"] = Headers("PEP-Info", "Used to provide information about a specific protocol extension.");
	_headers["permissions-policy"] = Headers("Permissions-Policy", "Allows and denies use of browser features.");
	_headers["pics-label"] = Headers("PICS-Label", "Used to indicate a PICS label for the resource.");
	_headers["ping-from"] = Headers("Ping-From", "Used in WebDAV to indicate the source of a ping.");
	_headers["ping-to"] = Headers("Ping-To", "Used in WebDAV to indicate the destination of a ping.");
	_headers["position"] = Headers("Position", "Used to indicate the position of a resource in a collection.");
	_headers["pragma"] = Headers("Pragma", "Backwards compatibility header for HTTP/1.0.");
	_headers["prefer"] = Headers("Prefer", "Indicates preferences for specific server behaviors.");
	_headers["preference-applied"] = Headers("Preference-Applied", "Informs which preferences were applied.");
	_headers["priority"] = Headers("Priority", "Provides hint about the priority of a resource request.");
	_headers["profileobject"] = Headers("ProfileObject", "Used to indicate a profile object for the resource.");
	_headers["protocol"] = Headers("Protocol", "Used to indicate the protocol used in the message.");
	_headers["protocol-info"] = Headers("Protocol-Info", "Used to provide information about the protocol used in the message.");
	_headers["protocol-query"] = Headers("Protocol-Query", "Used to indicate a protocol query.");
	_headers["protocol-request"] = Headers("Protocol-Request", "Used to indicate a protocol request.");
	_headers["proxy-authenticate"] = Headers("Proxy-Authenticate", "Authentication method for resource behind proxy.");
	_headers["proxy-authentication-info"] = Headers("Proxy-Authentication-Info", "Additional proxy authentication information.");
	_headers["proxy-authorization"] = Headers("Proxy-Authorization", "Credentials to authenticate with proxy server.");
	_headers["proxy-features"] = Headers("Proxy-Features", "Used to indicate features supported by a proxy.");
	_headers["proxy-instruction"] = Headers("Proxy-Instruction", "Used to indicate instructions for a proxy.");
	_headers["proxy-status"] = Headers("Proxy-Status", "Proxy response status information.");
	_headers["public"] = Headers("Public", "Used in WebDAV to indicate supported methods on a resource.");
	_headers["public-key-pins"] = Headers("Public-Key-Pins", "Used to indicate public key pinning policy.");
	_headers["public-key-pins-report-only"] = Headers("Public-Key-Pins-Report-Only", "Used to indicate public key pinning policy without enforcing it.");
	_headers["range"] = Headers("Range", "Indicates the part of a document that the server should return.");
	_headers["redirect-ref"] = Headers("Redirect-Ref", "Used to indicate the URI to redirect to.");
	_headers["referer"] = Headers("Referer", "Address of the previous web page from which link was followed.");
	_headers["referer-root"] = Headers("Referer-Root", "Used to indicate the root URI for referer information.");
	_headers["referrer-policy"] = Headers("Referrer-Policy", "Governs which referrer information should be included.");
	_headers["refresh"] = Headers("Refresh", "Directs the browser to reload or redirect.");
	_headers["repeatability-client-id"] = Headers("Repeatability-Client-ID", "Used to indicate a client-generated ID for repeatable requests.");
	_headers["repeatability-first-sent"] = Headers("Repeatability-First-Sent", "Used to indicate the time at which a repeatable request was first sent.");
	_headers["repeatability-request-id"] = Headers("Repeatability-Request-ID", "Used to indicate a server-generated ID for repeatable requests.");
	_headers["repeatability-result"] = Headers("Repeatability-Result", "Used to indicate the result of a repeatable request.");
	_headers["replay-nonce"] = Headers("Replay-Nonce", "Used to indicate a nonce for replay protection.");
	_headers["report-to"] = Headers("Report-To", "Defines endpoints for reporting errors and warnings.");
	_headers["reporting-endpoints"] = Headers("Reporting-Endpoints", "Used to indicate endpoints for reporting various types of information.");
	_headers["repr-digest"] = Headers("Repr-Digest", "Provides digest of the selected representation.");
	_headers["retry-after"] = Headers("Retry-After", "Indicates how long to wait before follow-up request.");
	_headers["rtt"] = Headers("RTT", "Indicates the estimated round trip time, in milliseconds.");

	// S
	_headers["safe"] = Headers("Safe", "Used to indicate that a request is safe to process.");
	_headers["save-data"] = Headers("Save-Data", "Indicates user preference for reduced data usage.");
	_headers["schedule-reply"] = Headers("Schedule-Reply", "Used in calendaring to indicate a reply to a schedule message.");
	_headers["schedule-tag"] = Headers("Schedule-Tag", "Used in calendaring to indicate a schedule tag for a resource.");
	_headers["sec-fetch-dest"] = Headers("Sec-Fetch-Dest", "Request destination in fetch metadata.");
	_headers["sec-fetch-mode"] = Headers("Sec-Fetch-Mode", "Request mode in fetch metadata.");
	_headers["sec-fetch-site"] = Headers("Sec-Fetch-Site", "Relationship between request and target origin.");
	_headers["sec-fetch-storage-access"] = Headers("Sec-Fetch-Storage-Access", "Indicates whether the request is for a resource that requires storage access.");
	_headers["sec-fetch-user"] = Headers("Sec-Fetch-User", "Whether navigation was triggered by user activation.");
	_headers["sec-gpc"] = Headers("Sec-GPC", "Indicates Global Privacy Control preference.");
	_headers["sec-purpose"] = Headers("Sec-Purpose", "Indicates purpose of the request.");
	_headers["sec-token-binding"] = Headers("Sec-Token-Binding", "Token binding identifier.");
	_headers["sec-websocket-accept"] = Headers("Sec-WebSocket-Accept", "Server willingness to upgrade to WebSocket.");
	_headers["sec-websocket-extensions"] = Headers("Sec-WebSocket-Extensions", "WebSocket extensions supported or selected.");
	_headers["sec-websocket-key"] = Headers("Sec-WebSocket-Key", "Key verifying WebSocket client intent.");
	_headers["sec-websocket-protocol"] = Headers("Sec-WebSocket-Protocol", "Sub-protocols supported or selected.");
	_headers["sec-websocket-version"] = Headers("Sec-WebSocket-Version", "WebSocket protocol version used by client.");
	_headers["security-scheme"] = Headers("Security-Scheme", "Used to indicate a security scheme for the resource.");
	_headers["server"] = Headers("Server", "Information about the origin server software.");
	_headers["server-timing"] = Headers("Server-Timing", "Metrics and descriptions for request-response cycle.");
	_headers["service-worker"] = Headers("Service-Worker", "Included in fetches for a service worker script resource.");
	_headers["service-worker-allowed"] = Headers("Service-Worker-Allowed", "Removes the path restriction for service worker scripts.");
	_headers["service-worker-navigation-preload"] = Headers("Service-Worker-Navigation-Preload", "Indicates a preloaded request during service worker startup.");
	_headers["set-cookie"] = Headers("Set-Cookie", "Send cookies from the server to the user-agent.");
	_headers["set-cookie2"] = Headers("Set-Cookie2", "Obsolete header that was used to send cookies from the server to the user-agent.");
	_headers["set-login"] = Headers("Set-Login", "Sets a federated identity provider login status.");
	_headers["set-txn"] = Headers("Set-Txn", "Used to indicate a transaction identifier for the resource.");
	_headers["setprofile"] = Headers("SetProfile", "Used to indicate a profile to be used in a response.");
	_headers["signature"] = Headers("Signature", "Conveys list of signatures for an exchange.");
	_headers["signature-input"] = Headers("Signature-Input", "Identifies response headers included in a signature.");
	_headers["signed-headers"] = Headers("Signed-Headers", "Identifies response headers included in a signature.");
	_headers["slug"] = Headers("SLUG", "Used in WebDAV to indicate a suggested name for the resource.");
	_headers["soapaction"] = Headers("SoapAction", "Used to indicate the intent of a SOAP HTTP request.");
	_headers["sourcemap"] = Headers("SourceMap", "Links to a source map for transformed code.");
	_headers["speculation-rules"] = Headers("Speculation-Rules", "Provides URLs for speculation rule JSON resources.");
	_headers["status-uri"] = Headers("Status-URI", "Used to indicate a URI for status information about the resource.");
	_headers["strict-transport-security"] = Headers("Strict-Transport-Security", "Forces communication using HTTPS.");
	_headers["supports-loading-mode"] = Headers("Supports-Loading-Mode", "Opts in to higher-risk loading modes.");
	_headers["sunset"] = Headers("Sunset", "Indicates when a resource will be deprecated.");
	_headers["surrogate-capability"] = Headers("Surrogate-Capability", "Used to indicate surrogate capabilities.");
	_headers["surrogate-control"] = Headers("Surrogate-Control", "Used to indicate surrogate control directives.");
	_headers["sec-browsing-topics"] = Headers("Sec-Browsing-Topics", "Sends the selected browsing topics for the current user.");
	_headers["sec-ch-device-memory"] = Headers("Sec-CH-Device-Memory", "Approximate amount of available client RAM memory.");
	_headers["sec-ch-dpr"] = Headers("Sec-CH-DPR", "Client device pixel ratio.");
	_headers["sec-ch-prefers-color-scheme"] = Headers("Sec-CH-Prefers-Color-Scheme", "User preference of dark or light color scheme.");
	_headers["sec-ch-prefers-reduced-motion"] = Headers("Sec-CH-Prefers-Reduced-Motion", "User preference to see fewer animations.");
	_headers["sec-ch-prefers-reduced-transparency"] = Headers("Sec-CH-Prefers-Reduced-Transparency", "User preference to reduce transparency.");
	_headers["sec-ch-ua"] = Headers("Sec-CH-UA", "User agent branding and version.");
	_headers["sec-ch-ua-arch"] = Headers("Sec-CH-UA-Arch", "User agent platform architecture.");
	_headers["sec-ch-ua-bitness"] = Headers("Sec-CH-UA-Bitness", "User agent CPU bitness.");
	_headers["sec-ch-ua-form-factors"] = Headers("Sec-CH-UA-Form-Factors", "User agent form-factors.");
	_headers["sec-ch-ua-full-version"] = Headers("Sec-CH-UA-Full-Version", "User agent full version string.");
	_headers["sec-ch-ua-full-version-list"] = Headers("Sec-CH-UA-Full-Version-List", "Full version for each brand in the user agent list.");
	_headers["sec-ch-ua-mobile"] = Headers("Sec-CH-UA-Mobile", "User agent is running on a mobile device.");
	_headers["sec-ch-ua-model"] = Headers("Sec-CH-UA-Model", "User agent device model.");
	_headers["sec-ch-ua-platform"] = Headers("Sec-CH-UA-Platform", "User agent underlying operating system.");
	_headers["sec-ch-ua-platform-version"] = Headers("Sec-CH-UA-Platform-Version", "User agent platform version.");
	_headers["sec-ch-ua-wow64"] = Headers("Sec-CH-UA-WoW64", "Whether the user agent binary is running in 32-bit mode on 64-bit Windows.");
	_headers["sec-ch-viewport-height"] = Headers("Sec-CH-Viewport-Height", "Client viewport height in CSS pixels.");
	_headers["sec-ch-viewport-width"] = Headers("Sec-CH-Viewport-Width", "Client viewport width in CSS pixels.");
	_headers["sec-ch-width"] = Headers("Sec-CH-Width", "Image width in CSS pixels.");
	_headers["sec-private-state-token"] = Headers("Sec-Private-State-Token", "Provides a private state token for the request.");
	_headers["sec-private-state-token-crypto-version"] = Headers("Sec-Private-State-Token-Crypto-Version", "Indicates the crypto version for private state tokens.");
	_headers["sec-private-state-token-lifetime"] = Headers("Sec-Private-State-Token-Lifetime", "Indicates lifetime for private state tokens.");
	_headers["sec-redemption-record"] = Headers("Sec-Redemption-Record", "Provides a redemption record for private state tokens.");
	_headers["sec-speculation-tags"] = Headers("Sec-Speculation-Tags", "Tags that resulted in speculation rule activation.");
	
	// T - U
	_headers["tcn"] = Headers("TCN", "Used to indicate the type of a connection.");
	_headers["te"] = Headers("TE", "Specifies transfer encodings the user agent will accept.");
	_headers["timeout"] = Headers("Timeout", "Used in WebDAV to specify timeout for locks.");
	_headers["timing-allow-origin"] = Headers("Timing-Allow-Origin", "Origins allowed to see Resource Timing API values.");
	_headers["tk"] = Headers("Tk", "Indicates tracking status applied to the corresponding request.");
	_headers["topic"] = Headers("Topic", "Used to indicate a topic for the resource.");
	_headers["traceparent"] = Headers("Traceparent", "Used to indicate tracing information for distributed tracing.");
	_headers["tracestate"] = Headers("Tracestate", "Used to indicate tracing system-specific information for distributed tracing.");
	_headers["trailer"] = Headers("Trailer", "Indicates additional fields at the end of chunked message.");
	_headers["transfer-encoding"] = Headers("Transfer-Encoding", "Form of encoding used to safely transfer resource.");
	_headers["ttl"] = Headers("TTL", "Used to indicate the time-to-live of a resource.");
	_headers["upgrade"] = Headers("Upgrade", "Upgrades connection to a different protocol.");
	_headers["upgrade-insecure-requests"] = Headers("Upgrade-Insecure-Requests", "Requests an encrypted and authenticated response.");
	_headers["urgency"] = Headers("Urgency", "Used to indicate the urgency of a resource request.");
	_headers["uri"] = Headers("URI", "Used to indicate a URI for the resource.");
	_headers["use-as-dictionary"] = Headers("Use-As-Dictionary", "Used to indicate that a resource should be used as a compression dictionary.");
	_headers["user-agent"] = Headers("User-Agent", "Characteristics of the requesting software user agent.");

	// V - W
	_headers["variant-vary"] = Headers("Variant-Vary", "Used to indicate the headers that determine the variant selection.");
	_headers["vary"] = Headers("Vary", "Determines how to match request headers for caching.");
	_headers["via"] = Headers("Via", "Added by proxies, forward and reverse.");
	_headers["viewport-width"] = Headers("Viewport-Width", "Deprecated hint for the viewport width.");
	_headers["width"] = Headers("Width", "Deprecated hint for resource width.");
	_headers["want-content-digest"] = Headers("Want-Content-Digest", "States wish for Content-Digest header.");
	_headers["want-digest"] = Headers("Want-Digest", "States wish for Digest header.");
	_headers["want-repr-digest"] = Headers("Want-Repr-Digest", "States wish for Repr-Digest header.");
	_headers["warning"] = Headers("Warning", "General warning about possible problems with the entity body.");
	_headers["www-authenticate"] = Headers("WWW-Authenticate", "Authentication method that should be used.");

	// X - Other Non-Standard
	_headers["x-content-type-options"] = Headers("X-Content-Type-Options", "Disables MIME sniffing.");
	_headers["x-dns-prefetch-control"] = Headers("X-DNS-Prefetch-Control", "Controls DNS prefetching behavior.");
	_headers["x-frame-options"] = Headers("X-Frame-Options", "Whether page can be rendered in a frame.");
	_headers["x-forwarded-for"] = Headers("X-Forwarded-For", "Identifies originating IP addresses of client.");
	_headers["x-forwarded-host"] = Headers("X-Forwarded-Host", "Identifies original host requested by client.");
	_headers["x-forwarded-proto"] = Headers("X-Forwarded-Proto", "Identifies protocol (HTTP/HTTPS) used by client.");
	_headers["x-permitted-cross-domain-policies"] = Headers("X-Permitted-Cross-Domain-Policies", "Controls use of cross-domain policy files.");
	_headers["x-powered-by"] = Headers("X-Powered-By", "Information about technology used by server.");
	_headers["x-robots-tag"] = Headers("X-Robots-Tag", "Controls indexing of a web page by search engines.");
	_headers["x-xss-protection"] = Headers("X-XSS-Protection", "Enables cross-site scripting filtering.");
}

/**
 * @brief [TODO:description]
 */
HeadersRegistry::~HeadersRegistry()
{
}

/**
 * @brief [TODO:description]
 */
HeadersRegistry::HeadersRegistry(const HeadersRegistry &rhs)
{
	(void)rhs;
}

/**
 * @brief [TODO:description]
 */
HeadersRegistry &HeadersRegistry::operator=(const HeadersRegistry &rhs)
{
	(void)rhs;
	return *this;
}

/**
 * @brief [TODO:description]
 */
HeadersRegistry &HeadersRegistry::getInstance()
{
	static HeadersRegistry instance;
	return instance;
}

/**
 * @brief [TODO:description]
 * @param name [TODO:description]
 * @return [TODO:description]
 */
Headers HeadersRegistry::getHeader(const std::string name) const
{
	std::string lowerName = name;
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

	std::map<std::string, Headers>::const_iterator it = _headers.find(lowerName);
	if (it == _headers.end())
		return Headers();
	return it->second;
}

} // !headers
} // !http
