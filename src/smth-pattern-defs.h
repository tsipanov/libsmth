
/*
 * UrlPattern
 *
 * define a pattern that can be used by the Client to make semantically
 * valid Fragment Requests for the Presentation.
 *
 * +Bitrate:
 *	A placeholder expression for the Bit Rate of a Track.
 * +CustomAttributes:
 *	A placeholder expression for the Attributes used to
 *	disambiguate a Track from other Tracks in the Stream.
 * +TrackName:
 *	A unique identifier that applies to all Tracks in a Stream.
 * +Time:
 *	A placeholder expression for the time of a Fragment.
 */
char UrlPattern[] = \
	"QualityLevels({bitrate},{CustomAttributes})/Fragments(video={start_time})";

/*
 * FragmentRequest
 *
 * data required to request a Fragment from the Server.
 */

STRING_UINT32 $BITRATE
URISAFE_IDENTIFIER_NONNUMERIC $NAME /* The name of the Stream that contains the Requested Fragment. */
STRING_UINT64 $TIME /*  The time of the Requested Fragment. */
URISAFE_IDENTIFIER_NONNUMERIC $KEY /* The name of the Attribute */
URISAFE_IDENTIFIER $VALUE /* The value of the Attribute */

char FragmentRequest[] = \
	"/QualityLevels($BITRATE,$KEY=$VALUE))/Fragments($NAME=$TIME)";

