// TODO: don't forget header

/**
* @file HTTPHeadersRegistry.cpp
* @brief [TODO:description]
*/

#include <algorithm>
#include <cctype>
#include <webserv/headers/HTTPHeadersRegistry.hpp>

namespace webserv
{
namespace HTTPheaders
{

/**
 * @brief [TODO:description]
 * @link https://www.iana.org/assignments/http-fields/http-fields.xhtml
 */
HTTPHeadersRegistry::HTTPHeadersRegistry()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.headers.headersregistry");
	_logger->setLevel(log42::logRecord::INFO);
	// A
	_headers["a-im"] = HTTPHeader("A-IM", "Used with Response code to indicate partial instance manipulations.");
	_headers["accept"] = HTTPHeader("Accept", "Informs the server about the types of data that can be sent back.");
	_headers["accept-additions"] = HTTPHeader("Accept-Additions", "Indicates acceptance of additions to the resource.");
	_headers["accept-ch"] = HTTPHeader("Accept-CH", "Servers advertise support for Client Hints using this header.");
	_headers["accept-charset"] = HTTPHeader("Accept-Charset", "Informs the server about which character sets can be used in the response.");
	_headers["accept-datetime"] = HTTPHeader("Accept-Datetime", "Informs the server about the preferred version of a resource based on datetime.");
	_headers["accept-encoding"] = HTTPHeader("Accept-Encoding", "The encoding algorithm that can be used on the resource sent back.");
	_headers["accept-features"] = HTTPHeader("Accept-Features", "Indicates which features the client supports.");
	_headers["accept-language"] = HTTPHeader("Accept-Language", "Informs the server about the human language the server is expected to send back.");
	_headers["accept-patch"] = HTTPHeader("Accept-Patch", "Advertises which media type the server is able to understand in a PATCH request.");
	_headers["accept-post"] = HTTPHeader("Accept-Post", "Advertises which media type the server is able to understand in a POST request.");
	_headers["accept-query"] = HTTPHeader("Accept-Query", "Advertises which media type the server is able to understand in a QUERY request.");
	_headers["accept-ranges"] = HTTPHeader("Accept-Ranges", "Indicates if the server supports range requests.");
	_headers["accept-signature"] = HTTPHeader("Accept-Signature", "Indicates intention to take advantage of any available signatures.");
	_headers["access-control"] = HTTPHeader("Access-Control", "Advertises which control features the server supports.");
	_headers["access-control-allow-credentials"] = HTTPHeader("Access-Control-Allow-Credentials", "Indicates whether the response can be exposed when credentials flag is true.");
	_headers["access-control-allow-headers"] = HTTPHeader("Access-Control-Allow-Headers", "Indicates which headers can be used in the actual request.");
	_headers["access-control-allow-methods"] = HTTPHeader("Access-Control-Allow-Methods", "Specifies the methods allowed when accessing the resource.");
	_headers["access-control-allow-origin"] = HTTPHeader("Access-Control-Allow-Origin", "Indicates whether the response can be shared.");
	_headers["access-control-expose-headers"] = HTTPHeader("Access-Control-Expose-Headers", "Indicates which headers can be exposed as part of the response.");
	_headers["access-control-max-age"] = HTTPHeader("Access-Control-Max-Age", "Indicates how long preflight request results can be cached.");
	_headers["access-control-request-headers"] = HTTPHeader("Access-Control-Request-Headers", "Used in preflight requests to indicate which headers will be used.");
	_headers["access-control-request-method"] = HTTPHeader("Access-Control-Request-Method", "Used in preflight requests to indicate which method will be used.");
	_headers["activate-storage-access"] = HTTPHeader("Activate-Storage-Access", "Indicates activation of storage access permission.");
	_headers["age"] = HTTPHeader("Age", "Time in seconds that the object has been in a proxy cache.");
	_headers["allow"] = HTTPHeader("Allow", "Lists the set of HTTP request methods supported by a resource.");
	_headers["alpn"] = HTTPHeader("ALPN", "Used to indicate which protocol should be used over TLS.");
	_headers["alt-svc"] = HTTPHeader("Alt-Svc", "Used to list alternate ways to reach this service.");
	_headers["alt-used"] = HTTPHeader("Alt-Used", "Used to identify the alternative service in use.");
	_headers["alternates"] = HTTPHeader("Alternates", "Used to indicate alternative versions of a resource.");
	_headers["amp-cache-transform"] = HTTPHeader("AMP-Cache-Transform", "Indicates transformations applied by AMP cache.");
	_headers["apply-to-redirect-ref"] = HTTPHeader("Apply-To-Redirect-Ref", "Indicates whether the referrer should be included in redirected requests.");
	_headers["authentication-control"] = HTTPHeader("Authentication-Control", "Controls authentication behavior.");
	_headers["authentication-info"] = HTTPHeader("Authentication-Info", "Provides authentication information in response to authentication.");
	_headers["authorization"] = HTTPHeader("Authorization", "Contains the credentials to authenticate a user-agent with a server.");
	_headers["attribution-reporting-eligible"] = HTTPHeader("Attribution-Reporting-Eligible", "Marks the response eligible for attribution reporting.");
	_headers["attribution-reporting-register-source"] = HTTPHeader("Attribution-Reporting-Register-Source", "Registers an attribution reporting source.");
	_headers["attribution-reporting-register-trigger"] = HTTPHeader("Attribution-Reporting-Register-Trigger", "Registers an attribution reporting trigger.");
	_headers["available-dictionary"] = HTTPHeader("Available-Dictionary", "Indicates which dictionaries are available for compression.");
	
	// B - C
	_headers["c-ext"] = HTTPHeader("C-Ext", "Used to indicate support for experimental features.");
	_headers["c-man"] = HTTPHeader("C-Man", "Used to indicate the presence of an experimental feature.");
	_headers["c-opt"] = HTTPHeader("C-Opt", "Used to indicate support for optional features.");
	_headers["c-pep"] = HTTPHeader("C-PEP", "Used to indicate support for a specific protocol extension.");
	_headers["c-pep-info"] = HTTPHeader("C-PEP-Info", "Used to provide information about a specific protocol extension.");
	_headers["cache-control"] = HTTPHeader("Cache-Control", "Directives for caching mechanisms in both requests and responses.");
	_headers["cache-group-invalidation"] = HTTPHeader("Cache-Group-Invalidation", "Indicates that a cache group is invalidated.");
	_headers["cache-groups"] = HTTPHeader("Cache-Groups", "Indicates the cache groups that a response belongs to.");
	_headers["cache-status"] = HTTPHeader("Cache-Status", "Indicates cache status of the response.");
	_headers["cal-managed-id"] = HTTPHeader("Cal-Managed-ID", "Used in calendaring to indicate managed identifiers.");
	_headers["caldav-timezones"] = HTTPHeader("CalDAV-Timezones", "Used in calendaring to indicate timezone information.");
	_headers["capsule-protocol"] = HTTPHeader("Capsule-Protocol", "Used to indicate support for capsule protocols.");
	_headers["cdn-cache-control"] = HTTPHeader("CDN-Cache-Control", "Cache directives targeted at content delivery networks.");
	_headers["cdn-loop"] = HTTPHeader("CDN-Loop", "Used for loop detection in Content Delivery Networks.");
	_headers["cert-not-after"] = HTTPHeader("Cert-Not-After", "Indicates the certificate validity end time.");
	_headers["cert-not-before"] = HTTPHeader("Cert-Not-Before", "Indicates the certificate validity start time.");
	_headers["clear-site-data"] = HTTPHeader("Clear-Site-Data", "Instructs the user agent to clear browsing data.");
	_headers["client-cert"] = HTTPHeader("Client-Cert", "Client certificate as a header field.");
	_headers["client-cert-chain"] = HTTPHeader("Client-Cert-Chain", "Client certificate chain as a header field.");
	_headers["close"] = HTTPHeader("Close", "Indicates that the connection should be closed after completion of the response.");
	_headers["cmcd-object"] = HTTPHeader("CMCD-Object", "Used in Common Media Client Data to provide object-level information.");
	_headers["cmcd-request"] = HTTPHeader("CMCD-Request", "Used in Common Media Client Data to provide request-level information.");
	_headers["cmcd-session"] = HTTPHeader("CMCD-Session", "Used in Common Media Client Data to provide session-level information.");
	_headers["cmcd-status"] = HTTPHeader("CMCD-Status", "Used in Common Media Client Data to provide status information.");
	_headers["cmsd-dynamic"] = HTTPHeader("CMSD-Dynamic", "Used in Common Media Server Data to indicate dynamic information.");
	_headers["cmsd-static"] = HTTPHeader("CMSD-Static", "Used in Common Media Server Data to indicate static information.");
	_headers["concealed-auth-export"] = HTTPHeader("Concealed-Auth-Export", "Used to indicate that authentication information is concealed.");
	_headers["configuration-context"] = HTTPHeader("Configuration-Context", "Used to indicate the context of a configuration.");
	_headers["connection"] = HTTPHeader("Connection", "Controls whether the connection stays open after the transaction.");
	_headers["content-base"] = HTTPHeader("Content-Base", "Deprecated header that specifies the base URI for resolving relative URLs in the response.");
	_headers["content-digest"] = HTTPHeader("Content-Digest", "Provides a digest of the message content.");
	_headers["content-disposition"] = HTTPHeader("Content-Disposition", "Indicates if resource should be displayed inline or as download.");
	_headers["content-encoding"] = HTTPHeader("Content-Encoding", "Used to specify the compression algorithm.");
	_headers["content-id"] = HTTPHeader("Content-ID", "Used to identify a specific part of a multipart message.");
	_headers["content-language"] = HTTPHeader("Content-Language", "Describes the human language(s) intended for the audience.");
	_headers["content-length"] = HTTPHeader("Content-Length", "The size of the resource in decimal number of bytes.");
	_headers["content-location"] = HTTPHeader("Content-Location", "Indicates an alternate location for the returned data.");
	_headers["content-md5"] = HTTPHeader("Content-MD5", "A Base64-encoded MD5 digest of the message content.");
	_headers["content-range"] = HTTPHeader("Content-Range", "Indicates where in a full body a partial message belongs.");
	_headers["content-script-type"] = HTTPHeader("Content-Script-Type", "Used to specify the scripting language of the resource.");
	_headers["content-security-policy"] = HTTPHeader("Content-Security-Policy", "Controls resources the user agent is allowed to load.");
	_headers["content-security-policy-report-only"] = HTTPHeader("Content-Security-Policy-Report-Only", "Allows monitoring CSP violations without enforcing.");
	_headers["content-style-type"] = HTTPHeader("Content-Style-Type", "Used to specify the styling language of the resource.");
	_headers["content-type"] = HTTPHeader("Content-Type", "Indicates the media type of the resource.");
	_headers["content-version"] = HTTPHeader("Content-Version", "Used to indicate the version of the content.");
	_headers["cookie"] = HTTPHeader("Cookie", "Contains stored HTTP cookies previously sent by the server.");
	_headers["cookie2"] = HTTPHeader("Cookie2", "Obsolete header that was used to indicate support for the newer cookie specification.");
	_headers["cross-origin-embedder-policy"] = HTTPHeader("Cross-Origin-Embedder-Policy", "Declares an embedder policy for a document.");
	_headers["cross-origin-embedder-policy-report-only"] = HTTPHeader("Cross-Origin-Embedder-Policy-Report-Only", "Declares an embedder policy for a document without enforcing it.");
	_headers["cross-origin-opener-policy"] = HTTPHeader("Cross-Origin-Opener-Policy", "Prevents other domains from opening/controlling a window.");
	_headers["cross-origin-opener-policy-report-only"] = HTTPHeader("Cross-Origin-Opener-Policy-Report-Only", "Prevents other domains from opening/controlling a window without enforcing it.");
	_headers["cross-origin-resource-policy"] = HTTPHeader("Cross-Origin-Resource-Policy", "Prevents other domains from reading the response.");
	_headers["cta-common-access-token"] = HTTPHeader("CTA-Common-Access-Token", "Used in Common Token Access to indicate a common access token.");

	// D - E
	_headers["dasl"] = HTTPHeader("DASL", "Used in WebDAV to indicate a DASL query.");
	_headers["date"] = HTTPHeader("Date", "Contains the date and time at which the message was originated.");
	_headers["dav"] = HTTPHeader("DAV", "Used in WebDAV to indicate compliance with DAV protocol.");
	_headers["default-style"] = HTTPHeader("Default-Style", "Used to indicate the default style for a resource.");
	_headers["delta-base"] = HTTPHeader("Delta-Base", "Used to indicate the base URI for delta encoding.");
	_headers["deprecation"] = HTTPHeader("Deprecation", "Indicates that the resource is deprecated.");
	_headers["depth"] = HTTPHeader("Depth", "Used in WebDAV requests to specify the depth of operation.");
	_headers["derived-from"] = HTTPHeader("Derived-From", "Used to indicate the source of a derived resource.");
	_headers["destination"] = HTTPHeader("Destination", "Used in WebDAV to specify the destination resource.");
	_headers["detached-jws"] = HTTPHeader("Detached-JWS", "Used to indicate a detached JSON Web Signature.");
	_headers["differential-id"] = HTTPHeader("Differential-ID", "Used to indicate the identifier for a differential version of a resource.");
	_headers["dictionary-id"] = HTTPHeader("Dictionary-ID", "Used to indicate the identifier for a compression dictionary.");
	_headers["digest"] = HTTPHeader("Digest", "Used to indicate a digest of the message content.");
	_headers["dnt"] = HTTPHeader("DNT", "Indicates the user's tracking preference (Do Not Track).");
	_headers["dpop"] = HTTPHeader("DPoP", "Used to indicate a DPoP proof JWT for binding tokens to a TLS connection.");
	_headers["dpop-nonce"] = HTTPHeader("DPoP-Nonce", "Used to indicate a nonce for DPoP proof JWTs.");
	_headers["early-data"] = HTTPHeader("Early-Data", "Indicates the request has been conveyed in TLS early data.");
	_headers["ediint-features"] = HTTPHeader("EDIINT-Features", "Used to indicate support for EDIINT features.");
	_headers["etag"] = HTTPHeader("ETag", "A unique string identifying the version of the resource.");
	_headers["expect"] = HTTPHeader("Expect", "Indicates expectations that need to be fulfilled by the server.");
	_headers["expect-ct"] = HTTPHeader("Expect-CT", "Used to indicate that the site expects Certificate Transparency compliance.");
	_headers["expires"] = HTTPHeader("Expires", "The date/time after which the response is considered stale.");
	_headers["ext"] = HTTPHeader("Ext", "Used to indicate support for extensions to the protocol.");

	// F - G
	_headers["forwarded"] = HTTPHeader("Forwarded", "Contains information from the client-facing side of proxy servers.");
	_headers["from"] = HTTPHeader("From", "Contains an Internet email address for a human user.");
	_headers["getprofile"] = HTTPHeader("GetProfile", "Used to indicate a profile to be used in a GET request.");

	// H - I
	_headers["hobareg"] = HTTPHeader("Hobareg", "Used in WebDAV to indicate a resource that is a collection.");
	_headers["host"] = HTTPHeader("Host", "Specifies the domain name of the server (for virtual hosting).");
	_headers["http2-settings"] = HTTPHeader("HTTP2-Settings", "Used to indicate HTTP/2 connection parameters.");
	_headers["if"] = HTTPHeader("If", "Used in WebDAV for conditional operations.");
	_headers["if-match"] = HTTPHeader("If-Match", "Conditional request that applies only if resource matches ETags.");
	_headers["if-modified-since"] = HTTPHeader("If-Modified-Since", "Conditional request transmitting if resource modified after date.");
	_headers["if-none-match"] = HTTPHeader("If-None-Match", "Conditional request if resource doesn't match ETags.");
	_headers["if-range"] = HTTPHeader("If-Range", "Conditional range request fulfilled if etag or date matches.");
	_headers["if-schedule-tag-match"] = HTTPHeader("If-Schedule-Tag-Match", "Used in calendaring to indicate conditional request based on schedule tag.");
	_headers["if-unmodified-since"] = HTTPHeader("If-Unmodified-Since", "Conditional request if resource not modified after date.");
	_headers["idempotency-key"] = HTTPHeader("Idempotency-Key", "Provides a unique key for idempotent POST/PATCH requests.");
	_headers["im"] = HTTPHeader("IM", "Instance manipulation used with response codes.");
	_headers["include-referred-token-binding-id"] = HTTPHeader("Include-Referred-Token-Binding-ID", "Used to indicate that the referred token binding ID should be included.");
	_headers["integrity-policy"] = HTTPHeader("Integrity-Policy", "Ensures resources have Subresource Integrity guarantees.");
	_headers["integrity-policy-report-only"] = HTTPHeader("Integrity-Policy-Report-Only", "Reports resources that would violate integrity policy.");
	_headers["isolation"] = HTTPHeader("Isolation", "Used to indicate the isolation level of a resource.");

	// K - L
	_headers["keep-alive"] = HTTPHeader("Keep-Alive", "Controls how long a persistent connection should stay open.");
	_headers["label"] = HTTPHeader("Label", "Used to indicate a label for the resource.");
	_headers["last-event-id"] = HTTPHeader("Last-Event-ID", "Used in Server-Sent Events to indicate the ID of the last event received.");
	_headers["last-modified"] = HTTPHeader("Last-Modified", "The date and time the resource was last modified.");
	_headers["link"] = HTTPHeader("Link", "Provides a means for serializing one or more links in headers.");
	_headers["link-template"] = HTTPHeader("Link-Template", "Provides templated links.");
	_headers["location"] = HTTPHeader("Location", "Indicates the URL to redirect a page to.");
	_headers["lock-token"] = HTTPHeader("Lock-Token", "Used in WebDAV to indicate a lock token.");

	// M - N
	_headers["man"] = HTTPHeader("Man", "Used to indicate support for a specific protocol extension.");
	_headers["max-forwards"] = HTTPHeader("Max-Forwards", "Indicates maximum number of hops for TRACE method.");
	_headers["memento-datetime"] = HTTPHeader("Memento-Datetime", "Used for time-based access to resource states.");
	_headers["meter"] = HTTPHeader("Meter", "Used to indicate a metering requirement for the resource.");
	_headers["method-check"] = HTTPHeader("Method-Check", "Used to indicate that the server should check the method of the request.");
	_headers["method-check-expires"] = HTTPHeader("Method-Check-Expires", "Used to indicate the time after which the method check should be considered expired.");
	_headers["mime-version"] = HTTPHeader("MIME-Version", "Indicates the version of MIME used in the message.");
	_headers["negotiate"] = HTTPHeader("Negotiate", "Used to indicate support for content negotiation.");
	_headers["nel"] = HTTPHeader("NEL", "Defines a network error reporting policy.");
	_headers["no-vary-search"] = HTTPHeader("No-Vary-Search", "Defines how query parameters affect cache matching.");

	// O
	_headers["odata-entityid"] = HTTPHeader("OData-EntityId", "Used in OData to indicate the entity ID of a resource.");
	_headers["odata-isolation"] = HTTPHeader("OData-Isolation", "Used in OData to indicate the isolation level of a resource.");
	_headers["odata-maxversion"] = HTTPHeader("OData-MaxVersion", "Used in OData to indicate the maximum OData version supported by the client.");
	_headers["odata-version"] = HTTPHeader("OData-Version", "Used in OData to indicate the OData version of the message.");
	_headers["opt"] = HTTPHeader("Opt", "Used to indicate support for optional features.");
	_headers["optional-www-authenticate"] = HTTPHeader("Optional-WWW-Authenticate", "Used to indicate optional authentication methods.");
	_headers["ordering-type"] = HTTPHeader("Ordering-Type", "Used to indicate the ordering type of a resource.");
	_headers["observe-browsing-topics"] = HTTPHeader("Observe-Browsing-Topics", "Marks topics inferred from a request as observed.");
	_headers["origin"] = HTTPHeader("Origin", "Indicates where a fetch originates from.");
	_headers["origin-agent-cluster"] = HTTPHeader("Origin-Agent-Cluster", "Indicates origin-keyed agent cluster placement.");
	_headers["oscore"] = HTTPHeader("OSCORE", "Used to indicate support for OSCORE protocol.");
	_headers["oslc-core-version"] = HTTPHeader("OSLC-Core-Version", "Used to indicate the version of OSLC core supported.");
	_headers["overwrite"] = HTTPHeader("Overwrite", "Used in WebDAV to control overwrite on copy/move.");

	// P - R
	_headers["p3p"] = HTTPHeader("P3P", "Used to indicate the P3P policy of the server.");
	_headers["pep"] = HTTPHeader("PEP", "Used to indicate support for a specific protocol extension.");
	_headers["pep-info"] = HTTPHeader("PEP-Info", "Used to provide information about a specific protocol extension.");
	_headers["permissions-policy"] = HTTPHeader("Permissions-Policy", "Allows and denies use of browser features.");
	_headers["pics-label"] = HTTPHeader("PICS-Label", "Used to indicate a PICS label for the resource.");
	_headers["ping-from"] = HTTPHeader("Ping-From", "Used in WebDAV to indicate the source of a ping.");
	_headers["ping-to"] = HTTPHeader("Ping-To", "Used in WebDAV to indicate the destination of a ping.");
	_headers["position"] = HTTPHeader("Position", "Used to indicate the position of a resource in a collection.");
	_headers["pragma"] = HTTPHeader("Pragma", "Backwards compatibility header for HTTP/1.0.");
	_headers["prefer"] = HTTPHeader("Prefer", "Indicates preferences for specific server behaviors.");
	_headers["preference-applied"] = HTTPHeader("Preference-Applied", "Informs which preferences were applied.");
	_headers["priority"] = HTTPHeader("Priority", "Provides hint about the priority of a resource request.");
	_headers["profileobject"] = HTTPHeader("ProfileObject", "Used to indicate a profile object for the resource.");
	_headers["protocol"] = HTTPHeader("Protocol", "Used to indicate the protocol used in the message.");
	_headers["protocol-info"] = HTTPHeader("Protocol-Info", "Used to provide information about the protocol used in the message.");
	_headers["protocol-query"] = HTTPHeader("Protocol-Query", "Used to indicate a protocol query.");
	_headers["protocol-request"] = HTTPHeader("Protocol-Request", "Used to indicate a protocol request.");
	_headers["proxy-authenticate"] = HTTPHeader("Proxy-Authenticate", "Authentication method for resource behind proxy.");
	_headers["proxy-authentication-info"] = HTTPHeader("Proxy-Authentication-Info", "Additional proxy authentication information.");
	_headers["proxy-authorization"] = HTTPHeader("Proxy-Authorization", "Credentials to authenticate with proxy server.");
	_headers["proxy-features"] = HTTPHeader("Proxy-Features", "Used to indicate features supported by a proxy.");
	_headers["proxy-instruction"] = HTTPHeader("Proxy-Instruction", "Used to indicate instructions for a proxy.");
	_headers["proxy-status"] = HTTPHeader("Proxy-Status", "Proxy response status information.");
	_headers["public"] = HTTPHeader("Public", "Used in WebDAV to indicate supported methods on a resource.");
	_headers["public-key-pins"] = HTTPHeader("Public-Key-Pins", "Used to indicate public key pinning policy.");
	_headers["public-key-pins-report-only"] = HTTPHeader("Public-Key-Pins-Report-Only", "Used to indicate public key pinning policy without enforcing it.");
	_headers["range"] = HTTPHeader("Range", "Indicates the part of a document that the server should return.");
	_headers["redirect-ref"] = HTTPHeader("Redirect-Ref", "Used to indicate the URI to redirect to.");
	_headers["referer"] = HTTPHeader("Referer", "Address of the previous web page from which link was followed.");
	_headers["referer-root"] = HTTPHeader("Referer-Root", "Used to indicate the root URI for referer information.");
	_headers["referrer-policy"] = HTTPHeader("Referrer-Policy", "Governs which referrer information should be included.");
	_headers["refresh"] = HTTPHeader("Refresh", "Directs the browser to reload or redirect.");
	_headers["repeatability-client-id"] = HTTPHeader("Repeatability-Client-ID", "Used to indicate a client-generated ID for repeatable requests.");
	_headers["repeatability-first-sent"] = HTTPHeader("Repeatability-First-Sent", "Used to indicate the time at which a repeatable request was first sent.");
	_headers["repeatability-request-id"] = HTTPHeader("Repeatability-Request-ID", "Used to indicate a server-generated ID for repeatable requests.");
	_headers["repeatability-result"] = HTTPHeader("Repeatability-Result", "Used to indicate the result of a repeatable request.");
	_headers["replay-nonce"] = HTTPHeader("Replay-Nonce", "Used to indicate a nonce for replay protection.");
	_headers["report-to"] = HTTPHeader("Report-To", "Defines endpoints for reporting errors and warnings.");
	_headers["reporting-endpoints"] = HTTPHeader("Reporting-Endpoints", "Used to indicate endpoints for reporting various types of information.");
	_headers["repr-digest"] = HTTPHeader("Repr-Digest", "Provides digest of the selected representation.");
	_headers["retry-after"] = HTTPHeader("Retry-After", "Indicates how long to wait before follow-up request.");
	_headers["rtt"] = HTTPHeader("RTT", "Indicates the estimated round trip time, in milliseconds.");

	// S
	_headers["safe"] = HTTPHeader("Safe", "Used to indicate that a request is safe to process.");
	_headers["save-data"] = HTTPHeader("Save-Data", "Indicates user preference for reduced data usage.");
	_headers["schedule-reply"] = HTTPHeader("Schedule-Reply", "Used in calendaring to indicate a reply to a schedule message.");
	_headers["schedule-tag"] = HTTPHeader("Schedule-Tag", "Used in calendaring to indicate a schedule tag for a resource.");
	_headers["sec-fetch-dest"] = HTTPHeader("Sec-Fetch-Dest", "Request destination in fetch metadata.");
	_headers["sec-fetch-mode"] = HTTPHeader("Sec-Fetch-Mode", "Request mode in fetch metadata.");
	_headers["sec-fetch-site"] = HTTPHeader("Sec-Fetch-Site", "Relationship between request and target origin.");
	_headers["sec-fetch-storage-access"] = HTTPHeader("Sec-Fetch-Storage-Access", "Indicates whether the request is for a resource that requires storage access.");
	_headers["sec-fetch-user"] = HTTPHeader("Sec-Fetch-User", "Whether navigation was triggered by user activation.");
	_headers["sec-gpc"] = HTTPHeader("Sec-GPC", "Indicates Global Privacy Control preference.");
	_headers["sec-purpose"] = HTTPHeader("Sec-Purpose", "Indicates purpose of the request.");
	_headers["sec-token-binding"] = HTTPHeader("Sec-Token-Binding", "Token binding identifier.");
	_headers["sec-websocket-accept"] = HTTPHeader("Sec-WebSocket-Accept", "Server willingness to upgrade to WebSocket.");
	_headers["sec-websocket-extensions"] = HTTPHeader("Sec-WebSocket-Extensions", "WebSocket extensions supported or selected.");
	_headers["sec-websocket-key"] = HTTPHeader("Sec-WebSocket-Key", "Key verifying WebSocket client intent.");
	_headers["sec-websocket-protocol"] = HTTPHeader("Sec-WebSocket-Protocol", "Sub-protocols supported or selected.");
	_headers["sec-websocket-version"] = HTTPHeader("Sec-WebSocket-Version", "WebSocket protocol version used by client.");
	_headers["security-scheme"] = HTTPHeader("Security-Scheme", "Used to indicate a security scheme for the resource.");
	_headers["server"] = HTTPHeader("Server", "Information about the origin server software.");
	_headers["server-timing"] = HTTPHeader("Server-Timing", "Metrics and descriptions for request-response cycle.");
	_headers["service-worker"] = HTTPHeader("Service-Worker", "Included in fetches for a service worker script resource.");
	_headers["service-worker-allowed"] = HTTPHeader("Service-Worker-Allowed", "Removes the path restriction for service worker scripts.");
	_headers["service-worker-navigation-preload"] = HTTPHeader("Service-Worker-Navigation-Preload", "Indicates a preloaded request during service worker startup.");
	_headers["set-cookie"] = HTTPHeader("Set-Cookie", "Send cookies from the server to the user-agent.");
	_headers["set-cookie2"] = HTTPHeader("Set-Cookie2", "Obsolete header that was used to send cookies from the server to the user-agent.");
	_headers["set-login"] = HTTPHeader("Set-Login", "Sets a federated identity provider login status.");
	_headers["set-txn"] = HTTPHeader("Set-Txn", "Used to indicate a transaction identifier for the resource.");
	_headers["setprofile"] = HTTPHeader("SetProfile", "Used to indicate a profile to be used in a response.");
	_headers["signature"] = HTTPHeader("Signature", "Conveys list of signatures for an exchange.");
	_headers["signature-input"] = HTTPHeader("Signature-Input", "Identifies response headers included in a signature.");
	_headers["signed-headers"] = HTTPHeader("Signed-Headers", "Identifies response headers included in a signature.");
	_headers["slug"] = HTTPHeader("SLUG", "Used in WebDAV to indicate a suggested name for the resource.");
	_headers["soapaction"] = HTTPHeader("SoapAction", "Used to indicate the intent of a SOAP HTTP request.");
	_headers["sourcemap"] = HTTPHeader("SourceMap", "Links to a source map for transformed code.");
	_headers["speculation-rules"] = HTTPHeader("Speculation-Rules", "Provides URLs for speculation rule JSON resources.");
	_headers["status-uri"] = HTTPHeader("Status-URI", "Used to indicate a URI for status information about the resource.");
	_headers["strict-transport-security"] = HTTPHeader("Strict-Transport-Security", "Forces communication using HTTPS.");
	_headers["supports-loading-mode"] = HTTPHeader("Supports-Loading-Mode", "Opts in to higher-risk loading modes.");
	_headers["sunset"] = HTTPHeader("Sunset", "Indicates when a resource will be deprecated.");
	_headers["surrogate-capability"] = HTTPHeader("Surrogate-Capability", "Used to indicate surrogate capabilities.");
	_headers["surrogate-control"] = HTTPHeader("Surrogate-Control", "Used to indicate surrogate control directives.");
	_headers["sec-browsing-topics"] = HTTPHeader("Sec-Browsing-Topics", "Sends the selected browsing topics for the current user.");
	_headers["sec-ch-device-memory"] = HTTPHeader("Sec-CH-Device-Memory", "Approximate amount of available client RAM memory.");
	_headers["sec-ch-dpr"] = HTTPHeader("Sec-CH-DPR", "Client device pixel ratio.");
	_headers["sec-ch-prefers-color-scheme"] = HTTPHeader("Sec-CH-Prefers-Color-Scheme", "User preference of dark or light color scheme.");
	_headers["sec-ch-prefers-reduced-motion"] = HTTPHeader("Sec-CH-Prefers-Reduced-Motion", "User preference to see fewer animations.");
	_headers["sec-ch-prefers-reduced-transparency"] = HTTPHeader("Sec-CH-Prefers-Reduced-Transparency", "User preference to reduce transparency.");
	_headers["sec-ch-ua"] = HTTPHeader("Sec-CH-UA", "User agent branding and version.");
	_headers["sec-ch-ua-arch"] = HTTPHeader("Sec-CH-UA-Arch", "User agent platform architecture.");
	_headers["sec-ch-ua-bitness"] = HTTPHeader("Sec-CH-UA-Bitness", "User agent CPU bitness.");
	_headers["sec-ch-ua-form-factors"] = HTTPHeader("Sec-CH-UA-Form-Factors", "User agent form-factors.");
	_headers["sec-ch-ua-full-version"] = HTTPHeader("Sec-CH-UA-Full-Version", "User agent full version string.");
	_headers["sec-ch-ua-full-version-list"] = HTTPHeader("Sec-CH-UA-Full-Version-List", "Full version for each brand in the user agent list.");
	_headers["sec-ch-ua-mobile"] = HTTPHeader("Sec-CH-UA-Mobile", "User agent is running on a mobile device.");
	_headers["sec-ch-ua-model"] = HTTPHeader("Sec-CH-UA-Model", "User agent device model.");
	_headers["sec-ch-ua-platform"] = HTTPHeader("Sec-CH-UA-Platform", "User agent underlying operating system.");
	_headers["sec-ch-ua-platform-version"] = HTTPHeader("Sec-CH-UA-Platform-Version", "User agent platform version.");
	_headers["sec-ch-ua-wow64"] = HTTPHeader("Sec-CH-UA-WoW64", "Whether the user agent binary is running in 32-bit mode on 64-bit Windows.");
	_headers["sec-ch-viewport-height"] = HTTPHeader("Sec-CH-Viewport-Height", "Client viewport height in CSS pixels.");
	_headers["sec-ch-viewport-width"] = HTTPHeader("Sec-CH-Viewport-Width", "Client viewport width in CSS pixels.");
	_headers["sec-ch-width"] = HTTPHeader("Sec-CH-Width", "Image width in CSS pixels.");
	_headers["sec-private-state-token"] = HTTPHeader("Sec-Private-State-Token", "Provides a private state token for the request.");
	_headers["sec-private-state-token-crypto-version"] = HTTPHeader("Sec-Private-State-Token-Crypto-Version", "Indicates the crypto version for private state tokens.");
	_headers["sec-private-state-token-lifetime"] = HTTPHeader("Sec-Private-State-Token-Lifetime", "Indicates lifetime for private state tokens.");
	_headers["sec-redemption-record"] = HTTPHeader("Sec-Redemption-Record", "Provides a redemption record for private state tokens.");
	_headers["sec-speculation-tags"] = HTTPHeader("Sec-Speculation-Tags", "Tags that resulted in speculation rule activation.");
	
	// T - U
	_headers["tcn"] = HTTPHeader("TCN", "Used to indicate the type of a connection.");
	_headers["te"] = HTTPHeader("TE", "Specifies transfer encodings the user agent will accept.");
	_headers["timeout"] = HTTPHeader("Timeout", "Used in WebDAV to specify timeout for locks.");
	_headers["timing-allow-origin"] = HTTPHeader("Timing-Allow-Origin", "Origins allowed to see Resource Timing API values.");
	_headers["tk"] = HTTPHeader("Tk", "Indicates tracking status applied to the corresponding request.");
	_headers["topic"] = HTTPHeader("Topic", "Used to indicate a topic for the resource.");
	_headers["traceparent"] = HTTPHeader("Traceparent", "Used to indicate tracing information for distributed tracing.");
	_headers["tracestate"] = HTTPHeader("Tracestate", "Used to indicate tracing system-specific information for distributed tracing.");
	_headers["trailer"] = HTTPHeader("Trailer", "Indicates additional fields at the end of chunked message.");
	_headers["transfer-encoding"] = HTTPHeader("Transfer-Encoding", "Form of encoding used to safely transfer resource.");
	_headers["ttl"] = HTTPHeader("TTL", "Used to indicate the time-to-live of a resource.");
	_headers["upgrade"] = HTTPHeader("Upgrade", "Upgrades connection to a different protocol.");
	_headers["upgrade-insecure-requests"] = HTTPHeader("Upgrade-Insecure-Requests", "Requests an encrypted and authenticated response.");
	_headers["urgency"] = HTTPHeader("Urgency", "Used to indicate the urgency of a resource request.");
	_headers["uri"] = HTTPHeader("URI", "Used to indicate a URI for the resource.");
	_headers["use-as-dictionary"] = HTTPHeader("Use-As-Dictionary", "Used to indicate that a resource should be used as a compression dictionary.");
	_headers["user-agent"] = HTTPHeader("User-Agent", "Characteristics of the requesting software user agent.");

	// V - W
	_headers["variant-vary"] = HTTPHeader("Variant-Vary", "Used to indicate the headers that determine the variant selection.");
	_headers["vary"] = HTTPHeader("Vary", "Determines how to match request headers for caching.");
	_headers["via"] = HTTPHeader("Via", "Added by proxies, forward and reverse.");
	_headers["viewport-width"] = HTTPHeader("Viewport-Width", "Deprecated hint for the viewport width.");
	_headers["width"] = HTTPHeader("Width", "Deprecated hint for resource width.");
	_headers["want-content-digest"] = HTTPHeader("Want-Content-Digest", "States wish for Content-Digest header.");
	_headers["want-digest"] = HTTPHeader("Want-Digest", "States wish for Digest header.");
	_headers["want-repr-digest"] = HTTPHeader("Want-Repr-Digest", "States wish for Repr-Digest header.");
	_headers["warning"] = HTTPHeader("Warning", "General warning about possible problems with the entity body.");
	_headers["www-authenticate"] = HTTPHeader("WWW-Authenticate", "Authentication method that should be used.");

	// X - Other Non-Standard
	_headers["x-content-type-options"] = HTTPHeader("X-Content-Type-Options", "Disables MIME sniffing.");
	_headers["x-dns-prefetch-control"] = HTTPHeader("X-DNS-Prefetch-Control", "Controls DNS prefetching behavior.");
	_headers["x-frame-options"] = HTTPHeader("X-Frame-Options", "Whether page can be rendered in a frame.");
	_headers["x-forwarded-for"] = HTTPHeader("X-Forwarded-For", "Identifies originating IP addresses of client.");
	_headers["x-forwarded-host"] = HTTPHeader("X-Forwarded-Host", "Identifies original host requested by client.");
	_headers["x-forwarded-proto"] = HTTPHeader("X-Forwarded-Proto", "Identifies protocol (HTTP/HTTPS) used by client.");
	_headers["x-permitted-cross-domain-policies"] = HTTPHeader("X-Permitted-Cross-Domain-Policies", "Controls use of cross-domain policy files.");
	_headers["x-powered-by"] = HTTPHeader("X-Powered-By", "Information about technology used by server.");
	_headers["x-robots-tag"] = HTTPHeader("X-Robots-Tag", "Controls indexing of a web page by search engines.");
	_headers["x-xss-protection"] = HTTPHeader("X-XSS-Protection", "Enables cross-site scripting filtering.");
}

/**
 * @brief [TODO:description]
 */
HTTPHeadersRegistry::~HTTPHeadersRegistry() {}

/**
 * @brief [TODO:description]
 */
HTTPHeadersRegistry &HTTPHeadersRegistry::getInstance()
{
	static HTTPHeadersRegistry instance;
	return (instance);
}

/**
 * @brief [TODO:description]
 * @param name [TODO:description]
 * @return [TODO:description]
 */
HTTPHeader HTTPHeadersRegistry::getHeader(const std::string name) const
{
	std::string lowerName = name;
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

	std::map<std::string, HTTPHeader>::const_iterator it = _headers.find(lowerName);
	if (it == _headers.end())
		return HTTPHeader();
	return it->second;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	HTTPHeadersRegistry::getLogger() const
{
	return _logger;
}

} // !HTTPheaders
} // !webserv
