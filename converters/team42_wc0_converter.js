const {
    fromZigbeeConverters,
    toZigbeeConverters,
    exposes,
} = require('zigbee-herdsman-converters');

const {
    Buffer,
} = require('buffer');

function getKey(object, value, fallback, convertTo) {
    for (const key in object) {
        if (object[key]===value) {
            return convertTo ? convertTo(key) : key;
        }
    }

    return fallback;
}

function postfixWithEndpointName(value, msg, definition) {
    if (definition.meta && definition.meta.multiEndpoint) {
        const endpointName = definition.hasOwnProperty('endpoint') ?
            getKey(definition.endpoint(msg.device), msg.endpoint.ID) : msg.endpoint.ID;
        return `${value}_${endpointName}`;
    } else {
        return value;
    }
}

const bind = async (endpoint, target, clusters) => {
    for (const cluster of clusters) {
        await endpoint.bind(cluster, target);
    }
};

const ACCESS_STATE = 0b001;
const ACCESS_READ = 0b100;

const tz = {
    metering: {
        key: ['currentSummDelivered', 'multiplier', 'divisor', 'reset'],
        convertSet: async (entity, key, value, meta) => {
            let propertyName = key;
            const state = {};

            if (meta.endpoint_name) {
                propertyName = `${propertyName}_${meta.endpoint_name}`;
            }

            if (key === 'reset') {
                await entity.command('genBasic', 'resetFactDefault', {}, {});
                await entity.read('seMetering', ['divisor']);
                await entity.read('seMetering', ['multiplier']);
                await entity.read('seMetering', ['currentSummDelivered']);
                return {};
            }

            if (key === 'currentSummDelivered') {
                let buffer = new Buffer.alloc(8);
                const val = BigInt.asUintN(48, BigInt(value));
                buffer.writeBigUInt64LE(val);
                buffer = buffer.slice(0, 6);
                const lsb = buffer.readUInt32LE();
                const msb = buffer.readUInt16LE(4);

                await entity.write('seMetering', {0x0000: {value: [msb, lsb], type: 37}});
                await entity.read('seMetering', ['currentSummDelivered']);

                return {};
            }

            if (key === 'divisor') {
                value = parseInt(value);
                state[propertyName] = value;

                await entity.write('seMetering', {0x0302: {value: value, type: 34}});
                await entity.read('seMetering', ['divisor']);
                await entity.read('seMetering', ['currentSummDelivered']);
                return {state: state};
            }

            if (key === 'multiplier') {
                value = parseInt(value);
                state[propertyName] = value;
                await entity.write('seMetering', {0x0301: {value: value, type: 34}});
                await entity.read('seMetering', ['multiplier']);
                await entity.read('seMetering', ['currentSummDelivered']);
                return {state: state};
            }
        },
        convertGet: async (entity, key, meta) => {
            await entity.read('seMetering', [key]);
        },
    },
};

const fz = {
    metering: {
        cluster: 'seMetering',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            const payload = {};
            let multiplier = msg.endpoint.getClusterAttributeValue('seMetering', 'multiplier') || 1;
            let divisor = msg.endpoint.getClusterAttributeValue('seMetering', 'divisor') || 1;
            const summaFormatting = msg.endpoint.getClusterAttributeValue('seMetering', 'summaFormatting') || 251;

            if (msg.data.hasOwnProperty('multiplier')) {
                multiplier = parseInt(msg.data['multiplier']);
                payload[postfixWithEndpointName('multiplier', msg, model)] = multiplier;
            }

            if (msg.data.hasOwnProperty('divisor')) {
                divisor = parseInt(msg.data['divisor']);
                payload[postfixWithEndpointName('divisor', msg, model)] = divisor;
            }

            const factor = multiplier / divisor;

            if (msg.data.hasOwnProperty('currentSummDelivered')) {
                const rightNumbers = 0b00000111 & summaFormatting;
                const suppressLeadingZeros = summaFormatting & (1 << 7);
                const leftNumbers = summaFormatting >> 3;

                const data = msg.data['currentSummDelivered'];
                let value = data[0] * 0x100000000 * factor + data[1] * factor;

                value = parseFloat(value.toString()).toFixed(rightNumbers);
                value = Number(`${Math.round(Number(`${value}e${rightNumbers}`))}e-${rightNumbers}`);

                if (!suppressLeadingZeros) {
                    value = value.toString().padStart(leftNumbers, '0');
                }

                payload[postfixWithEndpointName('currentSummDelivered', msg, model)] = value;
            }

            if (msg.data.hasOwnProperty('unitOfMeasure')) {
                const unitOfMeasureLookup = {0: 'kWh', 1: 'm³', 2: 'ft3'};
                const value = msg.data['unitOfMeasure'];
                payload[postfixWithEndpointName('unitOfMeasure', msg, model)] = unitOfMeasureLookup[value];
            }

            return payload;
        },
    },
};

const device = {
    zigbeeModel: ['WC04'],
    model: 'WC04',
    vendor: 'Team42',
    description: '[TODO write description)',
    supports: '',
    fromZigbee: [fromZigbeeConverters.on_off, fz.metering],
    toZigbee: [toZigbeeConverters.on_off, tz.metering],
    endpoint: (device) => {
        return {'l1': 1, 'l2': 2, 'l3': 3, 'l4': 4};
    },
    meta: {configureKey: 1, multiEndpoint: true},
    configure: async (device, coordinatorEndpoint) => {
        for (let i = 0; i < 4; ++i ) {
            const endpoint = device.getEndpoint(i+1);
            await bind(endpoint, coordinatorEndpoint, ['genOnOff', 'seMetering']);

            await endpoint.configureReporting('genOnOff', [{
                attribute: 'onOff',
                minimumReportInterval: 0,
                maximumReportInterval: 3600,
                reportableChange: 0}]);

            await endpoint.read('seMetering', [
                'multiplier',
                'divisor',
                'summaFormatting',
                'unitOfMeasure',
                'status',
                'meteringDeviceType']);
        }
    },
    exposes: [
        exposes.switch().withState('state', true).withEndpoint('l1'),
        exposes.numeric('currentSummDelivered', ACCESS_STATE | ACCESS_READ).withEndpoint('l1'),
        exposes.text('unitOfMeasure', ACCESS_STATE).withEndpoint('l1'),
        exposes.numeric('multiplier', ACCESS_STATE | ACCESS_READ).withEndpoint('l1'),
        exposes.numeric('divisor', ACCESS_STATE | ACCESS_READ).withEndpoint('l1'),

        exposes.switch().withState('state', true).withEndpoint('l2'),
        exposes.text('unitOfMeasure', ACCESS_STATE).withEndpoint('l2'),
        exposes.numeric('currentSummDelivered', ACCESS_STATE | ACCESS_READ).withEndpoint('l2'),
        exposes.numeric('multiplier', ACCESS_STATE | ACCESS_READ).withEndpoint('l2'),
        exposes.numeric('divisor', ACCESS_STATE | ACCESS_READ).withEndpoint('l2'),

        exposes.switch().withState('state', true).withEndpoint('l3'),
        exposes.text('unitOfMeasure', ACCESS_STATE).withEndpoint('l3'),
        exposes.numeric('currentSummDelivered', ACCESS_STATE | ACCESS_READ).withEndpoint('l3'),
        exposes.numeric('multiplier', ACCESS_STATE | ACCESS_READ).withEndpoint('l3'),
        exposes.numeric('divisor', ACCESS_STATE | ACCESS_READ).withEndpoint('l3'),

        exposes.switch().withState('state', true).withEndpoint('l4'),
        exposes.text('unitOfMeasure', ACCESS_STATE).withEndpoint('l4'),
        exposes.numeric('currentSummDelivered', ACCESS_STATE | ACCESS_READ).withEndpoint('l4'),
        exposes.numeric('multiplier', ACCESS_STATE | ACCESS_READ).withEndpoint('l4'),
        exposes.numeric('divisor', ACCESS_STATE | ACCESS_READ).withEndpoint('l4'),
    ],
};

module.exports = device;
