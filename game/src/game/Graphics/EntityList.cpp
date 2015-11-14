#include "game/Graphics/EntityList.hpp"
#include "game/graphic.h"
#include "game/graphic_prt.h"
#include "game/char.h"

namespace Ego {
namespace Graphics {

int EntityList::element_t::cmp(const void * pleft, const void * pright)
{
	element_t * dleft = (element_t *)pleft;
	element_t * dright = (element_t *)pright;

	int   rv;
	float diff;

	diff = dleft->dist - dright->dist;

	if (diff < 0.0f)
	{
		rv = -1;
	}
	else if (diff > 0.0f)
	{
		rv = 1;
	}
	else
	{
		rv = 0;
	}

	return rv;
}

EntityList::EntityList() :
    _lst()
{
    //ctor
}

EntityList *EntityList::init()
{
    reset();
    return this;
}

gfx_rv EntityList::reset()
{
    // If there is nothing in the dolist, we are done.
    if (_lst.empty())
    {
        return gfx_success;
    }

    for(element_t &element : _lst)
    {
        // Tell all valid objects that they are removed from this dolist.
        if (INVALID_CHR_REF == element.ichr && element.iprt != INVALID_PRT_REF)
        {
            const std::shared_ptr<Ego::Particle> &pprt = ParticleHandler::get()[element.iprt];
            if (nullptr != pprt) pprt->inst.indolist = false;
        }
        else if (INVALID_PRT_REF == element.iprt && INVALID_CHR_REF != element.ichr)
        {
            const std::shared_ptr<Object> &pobj = _currentModule->getObjectHandler()[element.ichr];
            if (nullptr != pobj) pobj->inst.indolist = false;
        }
    }

    _lst.clear();
    return gfx_success;
}

gfx_rv EntityList::test_obj(const Object& obj)
{
    // The entity is not a candidate if the list is full.
    if (_lst.size() == CAPACITY)
    {
        return gfx_fail;
    }
    // The entity is not a candidate if it is not in game.
    if (!INGAME_PCHR(&obj))
    {
        return gfx_fail;
    }

    return gfx_success;
}

gfx_rv EntityList::add_obj_raw(Object& obj)
{
    /// @author ZZ
    /// @details This function puts an entity in the list

    // Don't add if it is hidden.
    if (obj.isHidden())
    {
        return gfx_fail;
    }

    // Don't add if it's in another character's inventory.
    if (_currentModule->getObjectHandler().exists(obj.inwhich_inventory))
    {
        return gfx_fail;
    }

    // Add!
    _lst.emplace_back(obj.getObjRef().get(), INVALID_PRT_REF);

    // Notify it that it is in a do list.
    obj.inst.indolist = true;

    // Add any weapons it is holding.
    Object *holding;
    holding = _currentModule->getObjectHandler().get(obj.holdingwhich[SLOT_LEFT]);
    if (holding && _lst.size() < CAPACITY)
    {
        add_obj_raw(*holding);
    }
    holding = _currentModule->getObjectHandler().get(obj.holdingwhich[SLOT_RIGHT]);
    if (holding && _lst.size() < CAPACITY)
    {
        add_obj_raw(*holding);
    }
    return gfx_success;
}

gfx_rv EntityList::test_prt(const std::shared_ptr<Ego::Particle>& prt)
{
    // The entity is not a candidate if the list is full.
    if (_lst.size() == CAPACITY)
    {
        return gfx_fail;
    }

    // The entity is not a candidate if it is not displayed.
    if (!prt || prt->isTerminated())
    {
        return gfx_fail;
    }

    // The entity is not a candidate if it is explicitly or implicitly hidden.
    if (prt->isHidden() || 0 == prt->size)
    {
        return gfx_fail;
    }

    return gfx_success;
}

gfx_rv EntityList::add_prt_raw(const std::shared_ptr<Ego::Particle>& prt)
{
    /// @author ZZ
    /// @details This function puts an entity in the list

    _lst.emplace_back(INVALID_CHR_REF, prt->getParticleID());
    prt->inst.indolist = true;

    return gfx_success;
}

gfx_rv EntityList::sort(Camera& cam, const bool do_reflect)
{
    /// @author ZZ
    /// @details This function orders the entity list based on distance from camera,
    ///    which is needed for reflections to properly clip themselves.
    ///    Order from closest to farthest
    if (_lst.size() >= CAPACITY)
    {
        throw std::logic_error("invalid entity list size");
    }

	Vector3f vcam;
    mat_getCamForward(cam.getViewMatrix(), vcam);

    // Figure the distance of each.
    size_t count = 0;
    for (size_t i = 0; i < _lst.size(); ++i)
    {
		Vector3f vtmp;

        if (INVALID_PRT_REF == _lst[i].iprt && INVALID_CHR_REF != _lst[i].ichr)
        {
			Vector3f pos_tmp;

            CHR_REF iobj = _lst[i].ichr;

            if (do_reflect)
            {
                pos_tmp = mat_getTranslate(_currentModule->getObjectHandler().get(iobj)->inst.ref.matrix);
            }
            else
            {
                pos_tmp = mat_getTranslate(_currentModule->getObjectHandler().get(iobj)->inst.matrix);
            }

            vtmp = pos_tmp - cam.getPosition();
        }
        else if (INVALID_CHR_REF == _lst[i].ichr && _lst[i].iprt != INVALID_PRT_REF)
        {
            PRT_REF iprt = _lst[i].iprt;

            if (do_reflect)
            {
                vtmp = ParticleHandler::get()[iprt]->inst.pos - cam.getPosition();
            }
            else
            {
                vtmp = ParticleHandler::get()[iprt]->inst.ref_pos - cam.getPosition();
            }
        }
        else
        {
            continue;
        }

        float dist = vtmp.dot(vcam);
        if (dist > 0)
        {
            _lst[count].ichr = _lst[i].ichr;
            _lst[count].iprt = _lst[i].iprt;
            _lst[count].dist = dist;
            count++;
        }
    }

    // use qsort to sort the list in-place
    if (count > 1)
    {
        qsort(_lst.data(), _lst.size(), sizeof(element_t), element_t::cmp);
    }

    return gfx_success;
}

} // namespace Graphics
} // namespace Ego
