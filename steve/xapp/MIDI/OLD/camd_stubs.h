//	File		:	camd_stubs.h
//	Uses		:
//	Date		:	28-04-93
//	Author	:	Dan Baker, Commodore Business Machines
//	Desc.		:	Stub functions for the camd and the Realtime.library
//

/* Compiler glue: stub functions for camd.library */

struct MidiNode *CreateMidi(Tag tag, ...)
{
	return CreateMidiA((struct TagItem *)&tag );
}

BOOL SetMidiAttrs(struct MidiNode *mi, Tag tag, ...)
{
	return SetMidiAttrsA(mi, (struct TagItem *)&tag );
}

struct MidiLink *AddMidiLink(struct MidiNode *mi, LONG type, Tag tag, ...)
{
	return AddMidiLinkA(mi, type, (struct TagItem *)&tag );
}


/* Compiler glue: stub functions for realtime.library */
BOOL SetPlayerAttrs(struct PlayerInfo *pi, Tag tag, ...)
{
	return SetPlayerAttrsA(pi, (struct TagItem *)&tag );
}

struct PlayerInfo *CreatePlayer(Tag tag, ...)
{
	return CreatePlayerA( (struct TagItem *)&tag );
}

