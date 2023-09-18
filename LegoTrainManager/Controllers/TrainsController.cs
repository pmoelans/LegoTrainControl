using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using LegoTrainManager.Data;
using LegoTrainManager.Models;

namespace LegoTrainManager.Controllers
{
    public class TrainsController : Controller
    {
        private readonly ApplicationDbContext _context;
        private readonly IMQTTTrainControlHandler _mqttTrainControlHandler;

        public TrainsController(ApplicationDbContext context, IMQTTTrainControlHandler mqttTrainControlHandler)
        {
            _context = context;
            _mqttTrainControlHandler = mqttTrainControlHandler;

       
        }

        public async Task<IActionResult> RefreshView()
        {
           return RedirectToAction(nameof(Index));
        }
       
        public async Task<IActionResult> UpdateTrainState(TrainStatusClass trainStatus)
        {
            try
            {
                var existingTrain = await _context.Trains.FirstOrDefaultAsync(t => t.Name == trainStatus.TrainId);
                if (existingTrain != null)
                {
                    existingTrain.BatteryVoltage = trainStatus.Vbat;
                    existingTrain.LastConnectionTime = DateTime.Now;
                    existingTrain.Online = true;

                    await _context.SaveChangesAsync();
                   return await RefreshView();
                }

                var newTrain = new Train
                {
                    Name = trainStatus.TrainId,
                    BatteryVoltage = trainStatus.Vbat,
                    LastConnectionTime = DateTime.Now,
                    Online = true
                };
                await _context.Trains.AddAsync(newTrain);
                await _context.SaveChangesAsync();
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                throw;
            }
            return await RefreshView();
        }

        // GET: Trains
        public async Task<IActionResult> Index()
        {
            Response.Headers.Add("Refresh","5");

            await RemoveOldData();

            return _context.Trains != null ? 
                          View(await _context.Trains.Where(t=>t.LastConnectionTime>DateTime.Now.AddDays(-1)).ToListAsync()) :
                          Problem("Entity set 'ApplicationDbContext.Trains'  is null.");
        }

        private async Task RemoveOldData()
        {
            var trains = await _context.Trains.Where(t => t.LastConnectionTime < DateTime.UtcNow.AddDays(-1)).ToListAsync();
            _context.Trains.RemoveRange(trains);
            await _context.SaveChangesAsync();
        }
        public async Task<IActionResult> ShutDownAllTrains()
        {
            var trains = await _context.Trains.ToListAsync();

            foreach (var train in trains)
            {
                await _mqttTrainControlHandler.ShutDownTrain(train);
            }
            return RedirectToAction(nameof(Index));
        }


        public async Task<IActionResult> EmergencyStopAllTrains()
        {
            var trains = await _context.Trains.ToListAsync();

            foreach (var train in trains)
            {
                await _mqttTrainControlHandler.EstopTrain(train);

            }
            return RedirectToAction(nameof(Index));
        }

        // GET: Trains/Details/5
        public async Task<IActionResult> Details(int? id)
        {
            if (id == null || _context.Trains == null)
            {
                return NotFound();
            }

            var train = await _context.Trains
                .FirstOrDefaultAsync(m => m.Id == id);
            if (train == null)
            {
                return NotFound();
            }

            return View(train);
        }

        // GET: Trains/Create
        public IActionResult Create()
        {
            return View();
        }

        // POST: Trains/Create
        // To protect from overposting attacks, enable the specific properties you want to bind to.
        // For more details, see http://go.microsoft.com/fwlink/?LinkId=317598.
        [HttpPost]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> Create([Bind("Id,Name,BatteryVoltage,LastConnectionTime,Online,DeviceType")] Train train)
        {
            if (ModelState.IsValid)
            {
                _context.Add(train);
                await _context.SaveChangesAsync();
                return RedirectToAction(nameof(Index));
            }
            return View(train);
        }

        //[HttpPost("TrainStop/{id}")]
        public async Task<IActionResult> TrainControl([FromRoute]int? id, int speed)
        {

             //Find the train we are looking for:
             var train = await _context.Trains.FirstOrDefaultAsync(m => m.Id == id);

             if (train == null)
             {
                return RedirectToAction(nameof(Index));
             }
             
             await _context.SaveChangesAsync();

            await _mqttTrainControlHandler.UpdateTrainControl(train, speed);

            return RedirectToAction(nameof(Index));

        }


        // GET: Trains/Edit/5
        public async Task<IActionResult> Edit(int? id)
        {
            if (id == null || _context.Trains == null)
            {
                return NotFound();
            }

            var train = await _context.Trains.FindAsync(id);
            if (train == null)
            {
                return NotFound();
            }
            return View(train);
        }
        // GET: Trains/Edit/5
        //public async Task<IActionResult> Edit(Train train)
        //{
        //    //if (id == null || _context.Trains == null)
        //    //{
        //    //    return NotFound();
        //    //}

        //    var tr = await _context.Trains.FindAsync(train.Id);

        //    return View(train);
        //}

        [HttpPost]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> Edit(int id, [Bind("Id,Name,BatteryVoltage,LastConnectionTime, Online, Invert, DeviceType")] Train train)
        {
            if (id != train.Id)
            {
                return NotFound();
            }

            //if (ModelState.IsValid)
            //{
            
                try
                {
                    _context.Update(train);
                    await _context.SaveChangesAsync();
                }
                catch (DbUpdateConcurrencyException)
                {
                    throw;
                }
                return RedirectToAction(nameof(Index));
            //}
            //return View(train);
        }

        // GET: Trains/Delete/5
        public async Task<IActionResult> Delete(int? id)
        {
            if (id == null || _context.Trains == null)
            {
                return NotFound();
            }

            var train = await _context.Trains
                .FirstOrDefaultAsync(m => m.Id == id);
            if (train == null)
            {
                return NotFound();
            }

            return View(train);
        }

        // POST: Trains/Delete/5
        [HttpPost, ActionName("Delete")]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> DeleteConfirmed(int id)
        {
            if (_context.Trains == null)
            {
                return Problem("Entity set 'ApplicationDbContext.Trains'  is null.");
            }
            var train = await _context.Trains.FindAsync(id);
            if (train != null)
            {
                _context.Trains.Remove(train);
            }
            
            await _context.SaveChangesAsync();
            return RedirectToAction(nameof(Index));
        }

      
    }
}
