using System.Collections.Concurrent;
using LegoTrainManager.Models;
using Microsoft.EntityFrameworkCore;

namespace LegoTrainManager.Data
{
    public class TrainCatalog
    {
        public ConcurrentDictionary<string, Train> Trains { get; private set; } = new ConcurrentDictionary<string, Train>();

        public TrainCatalog()
        {

        }


        public Train? GetTrainById(int id)
        {
            if (!Trains.Values.Any())
                return null;
           return Trains.Values.FirstOrDefault(t => t.Id == id);
        }

        public void InsertUpdateTrain(Train newValue)
        {
           Train? existingValue;
           if( Trains.TryGetValue(newValue.Name,out existingValue))
           {
               Trains.TryUpdate(newValue.Name, newValue, existingValue);


           }


        }

        public void InsertUpdateTrain(TrainStatusClass train)
        {
            var currentMaxId = 1;
            if (Trains.Values.Any())
            {
                currentMaxId=Trains.Values.Max(t => t.Id);
            }
           


            if (!Trains.ContainsKey(train.TrainId))
            {
                var currentTrain = new Train
                {
                    Id=currentMaxId+1,
                    Name = train.TrainId,
                    BatteryVoltage = train.Vbat,
                    LastConnectionTime = DateTime.Now,
                    Online = true
                };

                Trains.TryAdd(train.TrainId, currentTrain);

                return;

            }

            var existingtrain = Trains[train.TrainId];

            existingtrain.BatteryVoltage = train.Vbat;
            existingtrain.LastConnectionTime = DateTime.Now;
            existingtrain.Online = true;
            Trains.TryUpdate(train.TrainId, Trains[train.TrainId], existingtrain);
            return;

        }

        public List<Train> GetAllTrains()
        {
            return Trains.Values.ToList();
            
        }

    }


}
