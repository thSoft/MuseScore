//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: keysig.cpp 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "sym.h"
#include "staff.h"
#include "clef.h"
#include "keysig.h"
#include "segment.h"
#include "score.h"
#include "undo.h"

const char* keyNames[15] = {
      QT_TRANSLATE_NOOP("MuseScore", "G major, E minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Cb major, Ab minor"),
      QT_TRANSLATE_NOOP("MuseScore", "D major, B minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Gb major, Eb minor"),
      QT_TRANSLATE_NOOP("MuseScore", "A major, F# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Db major, Bb minor"),
      QT_TRANSLATE_NOOP("MuseScore", "E major, C# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Ab major, F minor"),
      QT_TRANSLATE_NOOP("MuseScore", "B major, G# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Eb major, C minor"),
      QT_TRANSLATE_NOOP("MuseScore", "F# major, D# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "Bb major, G minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C# major, A# minor"),
      QT_TRANSLATE_NOOP("MuseScore", "F major,  D minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C major, A minor")
      };

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
      _showCourtesy = true;
	_showNaturals = true;
      }

KeySig::KeySig(const KeySig& k)
   : Element(k)
      {
	_showCourtesy = k._showCourtesy;
	_showNaturals = k._showNaturals;
	foreach(KeySym* ks, k.keySymbols)
            keySymbols.append(new KeySym(*ks));
      _sig = k._sig;
      }

//---------------------------------------------------------
//   setCustom
//---------------------------------------------------------

void KeySig::setCustom(const QList<KeySym*>& symbols)
      {
      _sig.setCustomType(0);
      keySymbols = symbols;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void KeySig::addLayout(int sym, qreal x, int line)
      {
      KeySym* ks = new KeySym;
      ks->sym    = sym;
      ks->spos   = QPointF(x, qreal(line) * .5);
      keySymbols.append(ks);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout()
      {
      qreal _spatium = spatium();
      setbbox(QRectF());

      if(staff() && staff()->isTabStaff()) {     // no key sigs on TAB staves
            foreach(KeySym* ks, keySymbols)
                  delete ks;
            keySymbols.clear();
            return;
            }

      if (isCustom()) {
            foreach(KeySym* ks, keySymbols) {
                  ks->pos = ks->spos * _spatium;
                  addbbox(symbols[score()->symIdx()][ks->sym].bbox(magS()).translated(ks->pos));
                  }
            return;
            }

      foreach(KeySym* ks, keySymbols)
            delete ks;
      keySymbols.clear();

      int clef = 0;
      if (staff())
            clef = staff()->clef(segment());

      int t1   = _sig.accidentalType();
      int t2   = _sig.naturalType();
      qreal xo = 0.0;

      int accidentals = 0, naturals = 0;
      switch(qAbs(t1)) {
            case 7: accidentals = 0x7f; break;
            case 6: accidentals = 0x3f; break;
            case 5: accidentals = 0x1f; break;
            case 4: accidentals = 0xf;  break;
            case 3: accidentals = 0x7;  break;
            case 2: accidentals = 0x3;  break;
            case 1: accidentals = 0x1;  break;
            case 0: accidentals = 0;    break;
            default:
                  qDebug("illegal t1 key %d (t2=%d)\n", t1, t2);
                  break;
            }
      switch(qAbs(t2)) {
            case 7: naturals = 0x7f; break;
            case 6: naturals = 0x3f; break;
            case 5: naturals = 0x1f; break;
            case 4: naturals = 0xf;  break;
            case 3: naturals = 0x7;  break;
            case 2: naturals = 0x3;  break;
            case 1: naturals = 0x1;  break;
            case 0: naturals = 0;    break;
            default:
                  qDebug("illegal t2 key %d (t1=%d)\n", t2, t1);
                  break;
            }

      xo = 0.0;
      int coffset = t2 < 0 ? 7 : 0;

      if (!((t1 > 0) ^ (t2 > 0)))
            naturals &= ~accidentals;

      if (_showNaturals) {
	      for (int i = 0; i < 7; ++i) {
                  if (naturals & (1 << i)) {
                        addLayout(naturalSym, xo, clefTable[clef].lines[i + coffset]);
				xo += 1.0;
				}
                  }
            }
      switch(t1) {
            case 7:  addLayout(sharpSym, xo + 6.0, clefTable[clef].lines[6]);
            case 6:  addLayout(sharpSym, xo + 5.0, clefTable[clef].lines[5]);
            case 5:  addLayout(sharpSym, xo + 4.0, clefTable[clef].lines[4]);
            case 4:  addLayout(sharpSym, xo + 3.0, clefTable[clef].lines[3]);
            case 3:  addLayout(sharpSym, xo + 2.0, clefTable[clef].lines[2]);
            case 2:  addLayout(sharpSym, xo + 1.0, clefTable[clef].lines[1]);
            case 1:  addLayout(sharpSym, xo + 0.0, clefTable[clef].lines[0]);
                     break;
            case -7: addLayout(flatSym, xo + 6.0, clefTable[clef].lines[13]);
            case -6: addLayout(flatSym, xo + 5.0, clefTable[clef].lines[12]);
            case -5: addLayout(flatSym, xo + 4.0, clefTable[clef].lines[11]);
            case -4: addLayout(flatSym, xo + 3.0, clefTable[clef].lines[10]);
            case -3: addLayout(flatSym, xo + 2.0, clefTable[clef].lines[9]);
            case -2: addLayout(flatSym, xo + 1.0, clefTable[clef].lines[8]);
            case -1: addLayout(flatSym, xo + 0.0, clefTable[clef].lines[7]);
            case 0:
                  break;
            default:
                  qDebug("illegal t1 key %d (t2=%d)\n", t1, t2);
                  break;
            }
      setbbox(QRectF());

      foreach(KeySym* ks, keySymbols) {
            ks->pos = ks->spos * _spatium;
            addbbox(symbols[score()->symIdx()][ks->sym].bbox(magS()).translated(ks->pos));
            }
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void KeySig::draw(QPainter* p) const
      {
      p->setPen(curColor());
      foreach(const KeySym* ks, keySymbols)
            symbols[score()->symIdx()][ks->sym].draw(p, magS(), QPointF(ks->pos.x(), ks->pos.y()));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == KEYSIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* KeySig::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() != KEYSIG) {
            delete e;
            return 0;
            }

      KeySig* ks    = static_cast<KeySig*>(e);
      KeySigEvent k = ks->keySigEvent();
      if (k.custom() && (score()->customKeySigIdx(ks) == -1))
            score()->addCustomKeySig(ks);
      else
            delete ks;
      if (data.modifiers & Qt::ControlModifier) {
            // apply only to this stave
            if (k != keySigEvent())
                  score()->undoChangeKeySig(staff(), tick(), k);
            }
      else {
            // apply to all staves:
            foreach(Staff* s, score()->staves())
                  score()->undoChangeKeySig(s, tick(), k);
            }
      return this;
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void KeySig::setSig(int old, int newSig)
      {
      KeySigEvent ks;
      ks.setNaturalType(old);
      ks.setAccidentalType(newSig);
      setKeySigEvent(ks);
      }

//---------------------------------------------------------
//   setOldSig
//---------------------------------------------------------

void KeySig::setOldSig(int old)
      {
      _sig.setNaturalType(old);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space KeySig::space() const
      {
      return Space(point(score()->styleS(ST_keysigLeftMargin)), width());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void KeySig::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      if (_sig.custom()) {
            xml.tag("custom", _sig.customType());
            }
      else {
            xml.tag("accidental", _sig.accidentalType());
            if (_sig.naturalType())
                  xml.tag("natural", _sig.naturalType());
            }
      foreach(const KeySym* ks, keySymbols) {
            xml.stag("KeySym");
            xml.tag("sym", ks->sym);
            xml.tag("pos", ks->spos);
            xml.etag();
            }
      xml.tag("showCourtesySig", _showCourtesy);
      xml.tag("showNaturals",    _showNaturals);
	xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void KeySig::read(const QDomElement& de)
      {
      _sig = KeySigEvent();   // invalidate _sig
      int subtype = 0;

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "KeySym") {
                  KeySym* ks = new KeySym;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        const QString& val(ee.text());
                        if (tag == "sym")
                              ks->sym = val.toInt();
                        else if (tag == "pos")
                              ks->spos = readPoint(ee);
                        else
                              domError(ee);
                        }
                  keySymbols.append(ks);
                  }
            else if (tag == "showCourtesySig")
		      _showCourtesy = val.toInt();
            else if (tag == "showNaturals")
		      _showNaturals = val.toInt();
            else if (tag == "accidental")
                  _sig.setAccidentalType(val.toInt());
            else if (tag == "natural")
                  _sig.setNaturalType(val.toInt());
            else if (tag == "custom")
                  _sig.setCustomType(val.toInt());
            else if (tag == "subtype")
                  subtype = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (_sig.invalid() && subtype) {
            _sig.initFromSubtype(subtype);     // for backward compatibility
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool KeySig::operator==(const KeySig& k) const
      {
      bool ct1 = customType() != 0;
      bool ct2 = k.customType() != 0;
      if (ct1 != ct2)
            return false;

      if (ct1) {
            int n = keySymbols.size();
            if (n != k.keySymbols.size())
                  return false;
            for (int i = 0; i < n; ++i) {
                  KeySym* ks1 = keySymbols[i];
                  KeySym* ks2 = k.keySymbols[i];
                  if (ks1->sym != ks2->sym)
                        return false;
                  if (ks1->spos != ks2->spos)
                        return false;
                  }
            return true;
            }
      return _sig == k._sig;
      }

//---------------------------------------------------------
//   changeKeySigEvent
//---------------------------------------------------------

void KeySig::changeKeySigEvent(const KeySigEvent& t)
      {
      if (_sig == t)
            return;
      if (t.custom()) {
            KeySig* ks = _score->customKeySig(t.customType());
            if (!ks)
                  return;
            foreach(KeySym* k, keySymbols)
                  delete k;
            keySymbols.clear();
            foreach(KeySym* k, ks->keySymbols)
                  keySymbols.append(new KeySym(*k));
            }
      setKeySigEvent(t);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int KeySig::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void KeySig::undoSetShowCourtesy(bool v)
      {
      score()->undoChangeProperty(this, P_SHOW_COURTESY, v);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant KeySig::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SHOW_COURTESY: return int(showCourtesy());
            case P_SHOW_NATURALS: return int(showNaturals());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool KeySig::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SHOW_COURTESY:
                  setShowCourtesy(v.toBool());
                  break;
            case P_SHOW_NATURALS:
                  setShowNaturals(v.toBool());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant KeySig::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_SHOW_COURTESY:      return true;
            case P_SHOW_NATURALS:      return true;
            default:                   return Element::propertyDefault(id);
            }
      }



