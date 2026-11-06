// Spell cards: their names, bonuses and music.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

// Each stage's music: its BGM numbers for App::PlayMidi, the first for
// the stage.
struct StageBgm
{
    int stageBgm;
    int bossBgm;
    int bossBgm2;
};
extern StageBgm g_StageBgms[];
// The music a practised spell card plays, by card number: entries up to
// and including lastCard play bgm. Ends with lastCard -1.
struct SpellCardBgm
{
    int lastCard;
    int bgm;
    char *midiPath;
    int titleIndex;
    int restartsMusic;
};
extern SpellCardBgm g_SpellCardBgms[];
// By stage and difficulty: what LoadingThread sets stats->timeOrbsNeeded to.
extern int g_TimeOrbsNeeded[][4];
// The difficulty (0 to 4) whose g_SpellCardLists list has spell card
// `card`, else 5.
int GetSpellCardDifficulty(int card);
// Whether spell card `card` is one of g_ListedSpellCards.
int IsListedSpellCard(int card);
// 43 spell cards (the 3 or 4 difficulties of several bosses' cards, and
// 171 to 190), and how many.
extern int g_ListedSpellCards[];
extern int g_ListedSpellCardCount;
// The spell card manager, at 0x4ea670 (SpellCard::RegisterChain
// registers it). Only what is known so far.
struct SpellCard
{
    // Constructs the timers and sprites (0x4143e0).
    SpellCard();

    // Bit 0 of flags. Inline; the copy at 0x4178a0 is in an /Od object.
    int IsActive()
    {
        return this->flags & 1;
    }
    // Initializes g_SpellCard and adds its update element (priority 0xc)
    // and draw element (priority 0xf), both allocated by
    // Chain::CreateElem. Returns 0, or -1 if Initialize fails (0x417f60).
    static int RegisterChain();
    // The chain callbacks: run Update and Draw (0x418010, 0x418030).
    static int OnUpdate(SpellCard *sc);
    static int OnDraw(SpellCard *sc);
    // Releases the boss faces (slots 0x12 and 0x13) unless restarting,
    // and the background and team faces (0xf to 0x11) while the scene
    // frees its files, then clears the update element's deleted
    // callback and removes the draw element. Returns 0 (0x418050).
    static int DeletedCallback(SpellCard *sc);
    // Removes the update element, which deletes the draw element too;
    // GameManager::DeletedCallback calls it before the stage's
    // (0x4180f0).
    static void RemoveChain();
    // The card's frame: bonus, timers, banners (0x416b90).
    int Update();
    // Draws the card's portraits, name and bonus (0x4178c0).
    int Draw();
    // Zeroes everything and loads (or, while the scene keeps its files,
    // takes) the dialogue background (slot 0xf), the two faces of the
    // player's team (slots 0x10 and 0x11, not while a card is practised)
    // and the boss faces of the stage or the practised card (slots 0x12
    // and 0x13, kept while restarting); then starts the sprites' scripts
    // and hides them. Returns 0, or -1 if loading fails (0x414590).
    int Initialize();
    // Starts spell card `card` named `name` for `enemy`, from an enemy
    // script instruction (StartSpellCard; th07's was the script's own
    // code) (0x4152a0).
    void Start(int card, char *name, int face, DWORD bonus, struct Enemy *enemy, char *text1, char *text2,
               char *text3);
    // Shows the card's name `name` with sprite `sprite`, and face `face`
    // unless it is negative (0x415f00).
    void ShowSpellCardName(int face, char *name, int sprite);
    // Ends the spell card, as th07's EndSpellCard did (0x4161b0).
    void End();
    // Starts the card named `name` (「リザレクション」) for `enemy` without a
    // capture: sets bit 10, clears bits 0 and 4 (0x415d10).
    void StartNameOnly(char *name, struct Enemy *enemy);
    // Ends the name banners ShowSpellCardName started: sets spellName's pending
    // interrupt directly and sends spellNameBar (and spellScoreBar unless
    // nameOnly) interrupt 2 (0x416160).
    void EndSpellCardName();
    // Hides the boss's card: clears active and ends the name banners
    // (0x416af0).
    void Abort();
    // Called with 8000 when a time orb is collected: unless bonusFrozen is
    // set, adds `amount` to the bonus, up to the bonus the card started
    // with, and below that `amount` / 120 to what it loses a second
    // (0x416b10).
    void AddBonus(int amount);
    // Shows the name of a bomb's spell card as the bomb starts
    // (StartBomb): the portrait for `face` (0 or 1, else none), `name`
    // and sprite `sprite` (0x415d60).
    void ShowBombName(int face, char *name, int sprite);
    // Bit 7 of flags: whether the player bombed during the spell card,
    // as th07's spellCardBombed? Inline; the copy is at 0x42dff0.
    int WasBombed()
    {
        return this->flags >> 7 & 1;
    }
    // Bit 9 of flags. Inline; the copy is at 0x405260.
    int IsCaptured()
    {
        return this->flags >> 9 & 1;
    }

    // Ends the spell card's bonus: sets bombName's pending interrupt
    // directly and sends bombNameBar interrupt 2. Inline; the copy at
    // 0x416130 is an /Od object's.
    void EndBombName()
    {
        this->bombName.pendingInterrupt = 1;
        this->bombNameBar.SetPendingInterrupt(2);
    }
    // Ends the spell card as a timer callback runs: clears capture and
    // bonus, sets timedOut. Inline; the copy at 0x42bc50 is an /Od
    // object's.
    void TimeOut()
    {
        this->capture = 0;
        this->timedOut = 1;
        this->bonus = 0;
    }
    // Clear capture and bonus; the first also copies bit 0 to bit 7.
    // Called as a bomb starts and as the player dies. Inline; the copies
    // at 0x44cba0 and 0x44d150 are the player object's.
    void LoseCaptureToBomb()
    {
        this->capture = 0;
        this->bonus = 0;
        this->bombed = this->active;
    }
    void LoseCapture()
    {
        this->capture = 0;
        this->bonus = 0;
    }

    union
    {
        DWORD flags;
        struct
        {
            DWORD active : 1;
            DWORD pad_0_1 : 1;
            DWORD capture : 1;
            DWORD timedOut : 1;
            DWORD unk_0_4 : 1;
            DWORD lastSpell : 1;
            DWORD effectFixed : 1;
            DWORD bombed : 1;
            // Set while a bonus is pending from the last card.
            DWORD bonusPending : 1;
            DWORD captured : 1;
            DWORD nameOnly : 1;
            DWORD bonusFrozen : 1;
        };
    };
    // timeOrbBonus. Inline; its copy is folded with ReplayManager::GetMode
    // (0x453cc0).
    int GetTimeOrbBonus()
    {
        return this->timeOrbBonus;
    }
    // Whether active and lastSpell are set. Inline; the copy is at
    // 0x417860.
    int IsLastSpellActive()
    {
        return this->IsActive() && this->lastSpell ? TRUE : FALSE;
    }
    // Whether active and capture are set. Inline; the copy is at
    // 0x41fd90.
    int IsCapturable()
    {
        return this->IsActive() && this->capture ? TRUE : FALSE;
    }
    // Set effectFixed and bonusFrozen (opcodes 164 and 184). Inline; the copies
    // at 0x41f0b0 and 0x41f0e0 are an /Od object's.
    void SetEffectFixed(int value)
    {
        this->effectFixed = value;
    }
    void SetBonusFrozen(int value)
    {
        this->bonusFrozen = value;
    }
    // Puts the card's effect at (x, y, z). Inline; the copy at 0x41f040
    // is an /Od object's.
    void SetEffectPos(float x, float y, float z);
    // timeLeft in frames. Inline; the copy is at 0x41fdd0.
    int GetTimeLeft()
    {
        return this->timeLeft.AsFrames();
    }

    // The boss whose card it is.
    struct Enemy *enemy;
    // The card's number.
    int card;
    // enemy->index.
    int enemyIndex;
    // Time orbs added as the card ends.
    int timeOrbBonus;
    // The card's name (decoded with 0xaa).
    char name[0x30];
    unsigned char pad_44[0x30];
    // Copied from Start's last two arguments while a card is practised.
    char text2[0x40];
    char text3[0x40];
    struct Effect *effect;
    // The ring effect after a capture, which End hands over from effect.
    struct Effect *captureEffect;
    // The bonus left, and what it loses each second.
    DWORD bonus;
    DWORD bonusStep;
    int capturedBonus;
    Timer timeLeft;
    Timer timeLimit;
    Sprite bombPortrait;
    Sprite spellPortrait;
    Sprite bombPortraitGlow;
    Sprite spellPortraitGlow;
    Sprite bombPortraitFrame;
    Sprite spellPortraitFrame;
    Sprite bombName;
    Sprite spellName;
    Sprite lastWordName1;
    Sprite lastWordName2;
    Sprite bombNameBar;
    Sprite spellNameBar;
    Sprite spellScoreDigit;
    Sprite spellScoreBar;
    float bombNameWidth;
    float spellNameWidth;
    D3DCOLOR nameColor;
    // The face ANMs (slots 0x10 to 0x13) of the dialogue's four portraits.
    // GuiImpl::StartMsgScript swaps the last two for stages 6B and Extra.
    AnmLoaded *faceAnms[4];
    // face_cdbg.anm (slot 0xf).
    AnmLoaded *backgroundAnm;
    // The bonus the card starts with.
    DWORD startBonus;
    ChainElem *calcElem;
    ChainElem *drawElem;
};
ASSERT_SIZE(SpellCard, 0x2644);
extern SpellCard g_SpellCard;
// Sets stats->checksumKeyPoints from stats->checksumKeyStage by difficulty
// (0x440470).
void SetChecksumKeyPoints();
// SetChecksumKeyPoints's tables: stats->checksumKeyPoints by
// stats->checksumKeyStage for the four main difficulties (then 500 more
// each), and for Extra and Phantasm (then 99999).
extern int g_ExtendPointItems[6];
extern int g_ExtendPointItemsExtra[4];
// g_SpellCardBgms' restartsMusic and titleIndex for the spell card
// practised; both ignore `card` (0x439916, 0x439961).
int GetPracticeBgmRestarts(int card);
int GetPracticeBgmTitleIndex(int card);
