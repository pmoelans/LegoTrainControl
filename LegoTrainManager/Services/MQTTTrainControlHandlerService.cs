using System.Configuration;
using LegoTrainManager.Controllers;
using LegoTrainManager.Data;
using LegoTrainManager.Models;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Server;

namespace LegoTrainManager.Services;

public class MQTTTrainControlHandlerService : IMQTTTrainControlHandler
{
    private readonly List<IMqttMessageHandler> _messageHandlers;
    private IConfiguration _config { get; }
    private MqttFactory _mqttFactory;
    private IMqttClient _mqttClient;


    public MQTTTrainControlHandlerService(IConfiguration configuration ): this(configuration, new List<IMqttMessageHandler> {})// new StatusHandler(dbContext) })
    {

    }


    private MQTTTrainControlHandlerService(IConfiguration config, List<IMqttMessageHandler> messageHandlers)
    {
        _messageHandlers = messageHandlers;
        _config = config;
        _mqttFactory = new MqttFactory();
        _mqttClient = _mqttFactory.CreateMqttClient();
    }



    private async Task ConnectAsync()
    {
        try
        {
            //using (var mqttClient = _mqttFactory.CreateMqttClient())
            //{
            // Use builder classes where possible in this project.
            var broker = _config.GetValue<string>("MQTTBroker");
            var clientId = _config.GetValue<string>("MQTTClientId");
            var mqttClientOptions = new MqttClientOptionsBuilder().
                WithTcpServer(broker).WithClientId(clientId).Build();

            // This will throw an exception if the server is not available.
            // The result from this message returns additional data which was sent 
            // from the server. Please refer to the MQTT protocol specification for details.
            var response = await _mqttClient.ConnectAsync(mqttClientOptions, CancellationToken.None);

            Console.WriteLine("The MQTT client is connected.");


            _mqttClient.ApplicationMessageReceivedAsync += e =>
            {
                async void Action(IMqttMessageHandler m) => await m.HandleMqttMessage(e.ApplicationMessage);

                _messageHandlers.ForEach(Action
                );
                return Task.CompletedTask;
            };
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
            throw;
        }

        

        //// Send a clean disconnect to the server by calling _DisconnectAsync_. Without this the TCP connection
        //// gets dropped and the server will handle this as a non clean disconnect (see MQTT spec for details).
        //var mqttClientDisconnectOptions = mqttFactory.CreateClientDisconnectOptionsBuilder().Build();

        //await mqttClient.DisconnectAsync(mqttClientDisconnectOptions, CancellationToken.None);
        // }
    }


    private void Subscribe()
    {
        var options = _mqttFactory.CreateSubscribeOptionsBuilder();

        _messageHandlers.ForEach(m => options.WithTopicFilter(m.Topic2Subscribe2));
        var builtOptions = options.Build();

        _mqttClient.SubscribeAsync(builtOptions, CancellationToken.None);
    }

    public async Task PublishAsync(string topic, int tractionPercentage)
    {
        var applicationMessage = new MqttApplicationMessageBuilder()
            .WithTopic(topic)
            .WithPayload(tractionPercentage.ToString())
            .Build();

        await _mqttClient.PublishAsync(applicationMessage, CancellationToken.None);



    }

    public async Task StartAsync(CancellationToken cancellationToken)
    {

        await ConnectAsync();

        //make all the subscriptions to the required topics;
        Subscribe();
    }

    public async Task StopAsync(CancellationToken cancellationToken)
    {
        await _mqttClient.DisconnectAsync();
    }
}