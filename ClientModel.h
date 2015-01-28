/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file ClientModel.h
 * @author Yann yann@ethdev.com
 * @author Arkadiy Paronyan arkadiy@ethdev.com
 * @date 2015
 * Ethereum IDE client.
 */

#pragma once

#include <atomic>
#include <map>
#include "DebuggingStateWrapper.h"
#include "MixClient.h"

using AssemblyDebuggerData = std::tuple<QList<QObject*>, dev::mix::QQMLMap*>;

Q_DECLARE_METATYPE(AssemblyDebuggerData)
Q_DECLARE_METATYPE(dev::mix::ExecutionResult)

namespace dev
{
namespace mix
{

class AppContext;
class Web3Server;
class RpcConnector;
class QEther;

/// Backend transaction config class
struct TransactionSettings
{
	TransactionSettings() {}
	TransactionSettings(QString const& _functionId, u256 _value, u256 _gas, u256 _gasPrice):
		functionId(_functionId), value(_value), gas(_gas), gasPrice(_gasPrice) {}
	TransactionSettings(u256 _value, u256 _gas, u256 _gasPrice):
		value(_value), gas(_gas), gasPrice(_gasPrice) {}
	TransactionSettings(QString const& _stdContractName, QString const& _stdContractUrl):
		functionId(_stdContractName), stdContractUrl(_stdContractUrl) {}

	/// Contract function name
	QString functionId;
	/// Transaction value
	u256 value;
	/// Gas
	u256 gas;
	/// Gas price
	u256 gasPrice;
	/// Mapping from contract function parameter name to value
	std::map<QString, u256> parameterValues;
	/// Standard contract url
	QString stdContractUrl;
};


/// UI Transaction log record
class TransactionLogEntry: public QObject
{
	Q_OBJECT
	/// Transaction block number
	Q_PROPERTY(unsigned block MEMBER m_block CONSTANT)
	/// Transaction index within the block
	Q_PROPERTY(unsigned index MEMBER m_index CONSTANT)
	/// Contract name if any
	Q_PROPERTY(QString contract MEMBER m_contract CONSTANT)
	/// Function name if any
	Q_PROPERTY(QString function MEMBER m_function CONSTANT)
	/// Transaction value
	Q_PROPERTY(QString value MEMBER m_value CONSTANT)
	/// Receiving address
	Q_PROPERTY(QString address MEMBER m_address CONSTANT)
	/// Returned value or transaction address in case of creation
	Q_PROPERTY(QString returned MEMBER m_returned CONSTANT)

public:
	TransactionLogEntry():
		m_block(0), m_index(0) {}
	TransactionLogEntry(int _block, int _index, QString _contract, QString _function, QString _value, QString _address, QString _returned):
		m_block(_block), m_index(_index), m_contract(_contract), m_function(_function), m_value(_value), m_address(_address), m_returned(_returned) {}

private:
	unsigned m_block;
	unsigned m_index;
	QString m_contract;
	QString m_function;
	QString m_value;
	QString m_address;
	QString m_returned;
};

/**
 * @brief Ethereum state control
 */
class ClientModel: public QObject
{
	Q_OBJECT

public:
	ClientModel(AppContext* _context);
	~ClientModel();
	/// @returns true if currently executing contract code
	Q_PROPERTY(bool running MEMBER m_running NOTIFY runStateChanged)
	/// @returns address of the last executed contract
	Q_PROPERTY(QString contractAddress READ contractAddress NOTIFY contractAddressChanged)
	/// ethereum.js RPC request entry point
	/// @param _message RPC request in Json format
	/// @returns RPC response in Json format
	Q_INVOKABLE QString apiCall(QString const& _message);

	/// Simulate mining. Creates a new block
	Q_INVOKABLE void mine();

public slots:
	/// Run the contract constructor and show debugger window.
	void debugDeployment();
	/// Setup state, run transaction sequence, show debugger for the last transaction
	/// @param _state JS object with state configuration
	void setupState(QVariantMap _state);
	/// Show the debugger for a specified transaction
	Q_INVOKABLE void debugTransaction(unsigned _block, unsigned _index);

private slots:
	/// Update UI with machine states result. Display a modal dialog.
	void showDebugger();
	/// Update UI with transaction run error.
	void showDebugError(QString const& _error);

signals:
	/// Transaction execution started
	void runStarted();
	/// Transaction execution completed successfully
	void runComplete();
	/// Transaction execution completed with error
	/// @param _message Error message
	void runFailed(QString const& _message);
	/// Contract address changed
	void contractAddressChanged();
	/// Execution state changed
	void newBlock();
	/// Execution state changed
	void runStateChanged();
	/// Show debugger window request
	void showDebuggerWindow();
	/// ethereum.js RPC response ready
	/// @param _message RPC response in Json format
	void apiResponse(QString const& _message);
	/// New transaction log entry
	void newTransaction(TransactionLogEntry* _tr);
	/// State (transaction log) cleared
	void stateCleared();

private:
	QString contractAddress() const;
	void executeSequence(std::vector<TransactionSettings> const& _sequence, u256 _balance);
	dev::Address deployContract(bytes const& _code, TransactionSettings const& _tr = TransactionSettings());
	void callContract(Address const& _contract, bytes const& _data, TransactionSettings const& _tr);
	void onNewTransaction();
	void onStateReset();
	void showDebuggerForTransaction(ExecutionResult const& _t);

	AppContext* m_context;
	std::atomic<bool> m_running;
	std::unique_ptr<MixClient> m_client;
	std::unique_ptr<RpcConnector> m_rpcConnector;
	std::unique_ptr<Web3Server> m_web3Server;
	Address m_contractAddress;
	std::map<QString, Address> m_stdContractAddresses;
	std::map<Address, QString> m_stdContractNames;
};

}
}
