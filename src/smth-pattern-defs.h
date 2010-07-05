
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




#if 0
The UrlPattern and related fields define a pattern that can be used by the client to make
semantically valid Fragment Requests for the presentation.
UrlPattern (variable): Encapsulates a pattern for constructing Fragment Requests.
BitrateSubstitution (variable): A placeholder expression for the Bit rate of a track.
CustomAttributesSubstitution (variable): A placeholder expression for the Attributes used to
disambiguate a track from other tracks in the stream.
TrackName (variable): A unique identifier that applies to all tracks in a stream.
BitrateSubstitution (variable): A placeholder expression for the time of a fragment.
The syntax of the fields defined in this section, specified in ABNF [RFC5234], is as follows:
   UrlPattern = QualityLevelsPattern "/" FragmentsPattern
   QualityLevelsPattern = QualityLevelsNoun "(" QualityLevelsPredicatePattern ")"
   QualityLevelsNoun = "QualityLevels"
   QualityLevelsPredicate = BitrateSubstitution ["," CustomAttributesSubstitution ]
   Bitrate = "{bitrate}" / "{Bitrate}"
   CustomAttributesSubstitution = "{CustomAttributes}"
   FragmentsPattern = FragmentsNoun "(" FragmentsPatternPredicate ")";
   FragmentsNoun = "Fragments"
   FragmentsPatternPredicate = TrackName "=" StartTimeSubstitution;
   TrackName = URISAFE_IDENTIFIER_NONNUMERIC
   StartTimeSubstitution = "{start time}" / "{start_time}"

P. 20 [24]
#endif

